/*
 * Software mixer. The audio port takes only 48 kHz
 * float blocks, and a missed block clicks, so one thread only copies finished blocks to the
 * port, and a second one mixes ahead of it.
 *
 * The game thread starts and stops voices, and the mixer thread ends them. Each field has one
 * writer, so voices need no lock.
 */
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <audio/audio.h>
#include <sys/event_queue.h>
#include <sys/thread.h>
#include <vorbis/vorbisfile.h>

#include "audio.h"
#include "wav.h"

#define OUTPUT_RATE       48000.0
#define BLOCK_FRAMES      AUDIO_BLOCK_SAMPLES
#define BLOCK_SAMPLES     (BLOCK_FRAMES * 2)
#define BLOCK_BYTES       (BLOCK_SAMPLES * sizeof(float))
#define PORT_BLOCKS       8
#define RING_BLOCKS       16
#define MAX_VOICES        24
#define STREAM_FRAMES     4096
#define THREAD_STACK      (128 * 1024)
#define FEEDER_PRIORITY   900
#define MIXER_PRIORITY    1000
#define WAIT_MICROSECONDS 20000

struct AudioClip {
    bool stream;
    float *frames;           /* WAV: whole sound, interleaved stereo */
    long frame_count;
    double rate;
    unsigned char *bytes;    /* Ogg: the compressed file */
    size_t size;
};

enum { VOICE_FREE, VOICE_STARTING, VOICE_PLAYING };

typedef struct {
    const AudioClip *clip;
    size_t position;
} MemoryFile;

typedef struct {
    volatile unsigned state;
    volatile unsigned generation;
    volatile unsigned stop_requested;
    float volume;
    bool loop;
    AudioClip *clip;
    double position;
    double rate;
    OggVorbis_File file;
    MemoryFile memory;
    bool file_open;
    bool file_ended;
    int channels;
    long buffer_start;
    int buffer_frames;
} Voice;

static Voice voices[MAX_VOICES];
static float stream_buffers[MAX_VOICES][STREAM_FRAMES * 2];
static float ring[RING_BLOCKS][BLOCK_SAMPLES] __attribute__((aligned(128)));
static volatile unsigned ring_write;
static volatile unsigned ring_read;
static volatile bool running;
static volatile bool paused;
static bool opened;
static audioPortConfig port_config;
static u32 port_number;
static sys_event_queue_t notify_queue;
static sys_ipc_key_t notify_key;
static sys_ppu_thread_t feeder_id;
static sys_ppu_thread_t mixer_id;
static unsigned underruns;

/* ------------------------------------------------------------------ Vorbis, from memory */

static size_t memory_read(void *target, size_t size, size_t count, void *source)
{
    MemoryFile *memory = source;
    size_t left = memory->clip->size - memory->position;
    size_t items = size == 0 ? 0 : left / size;

    if (items > count)
        items = count;
    memcpy(target, memory->clip->bytes + memory->position, items * size);
    memory->position += items * size;
    return items;
}

static int memory_seek(void *source, ogg_int64_t offset, int origin)
{
    MemoryFile *memory = source;
    ogg_int64_t position = offset;

    if (origin == SEEK_CUR)
        position += (ogg_int64_t)memory->position;
    else if (origin == SEEK_END)
        position += (ogg_int64_t)memory->clip->size;

    if (position < 0 || position > (ogg_int64_t)memory->clip->size)
        return -1;
    memory->position = (size_t)position;
    return 0;
}

static long memory_tell(void *source)
{
    return (long)((MemoryFile *)source)->position;
}

static const ov_callbacks MEMORY_CALLBACKS = { memory_read, memory_seek, NULL, memory_tell };

/* ------------------------------------------------------------------ one voice */

static void end_voice(Voice *voice)
{
    if (voice->file_open) {
        ov_clear(&voice->file);
        voice->file_open = false;
    }
    voice->stop_requested = 0;
    __asm__ volatile("lwsync");
    voice->state = VOICE_FREE;
}

static bool open_stream(Voice *voice)
{
    voice->memory.clip = voice->clip;
    voice->memory.position = 0;
    if (ov_open_callbacks(&voice->memory, &voice->file, NULL, 0, MEMORY_CALLBACKS) != 0)
        return false;

    voice->file_open = true;
    voice->channels = ov_info(&voice->file, -1)->channels;
    voice->rate = ov_info(&voice->file, -1)->rate;
    return true;
}

static bool begin_voice(Voice *voice)
{
    voice->position = 0.0;
    voice->buffer_start = 0;
    voice->buffer_frames = 0;
    voice->file_ended = false;

    if (voice->clip->stream)
        return open_stream(voice);
    voice->rate = voice->clip->rate;
    return true;
}

static void write_stereo(float *target, float **planes, int channels, long count)
{
    for (long i = 0; i < count; i++) {
        target[i * 2] = planes[0][i];
        target[i * 2 + 1] = planes[channels > 1 ? 1 : 0][i];
    }
}

/* Keeps the last frame, which the next read interpolates from, then fills the buffer behind it.
 * A looping stream goes back to its start when the file ends. */
static void refill_stream(Voice *voice, float *buffer)
{
    float **planes;
    int section;

    if (voice->buffer_frames > 0) {
        buffer[0] = buffer[(voice->buffer_frames - 1) * 2];
        buffer[1] = buffer[(voice->buffer_frames - 1) * 2 + 1];
        voice->buffer_start += voice->buffer_frames - 1;
        voice->buffer_frames = 1;
    }

    while (voice->buffer_frames < STREAM_FRAMES) {
        long got = ov_read_float(&voice->file, &planes, STREAM_FRAMES - voice->buffer_frames, &section);

        if (got <= 0 && voice->loop && ov_pcm_seek(&voice->file, 0) == 0)
            continue;
        if (got <= 0) {
            voice->file_ended = true;
            return;
        }
        write_stereo(buffer + voice->buffer_frames * 2, planes, voice->channels, got);
        voice->buffer_frames += (int)got;
    }
}

static bool mix_stream(Voice *voice, float *target, int index)
{
    float *buffer = stream_buffers[index];
    double step = voice->rate / OUTPUT_RATE;

    for (int i = 0; i < BLOCK_FRAMES; i++) {
        long whole = (long)voice->position;

        while (whole + 1 >= voice->buffer_start + voice->buffer_frames && !voice->file_ended)
            refill_stream(voice, buffer);

        long offset = whole - voice->buffer_start;
        if (offset < 0 || offset >= voice->buffer_frames)
            return false;

        long next = offset + 1 < voice->buffer_frames ? offset + 1 : offset;
        float blend = (float)(voice->position - (double)whole);

        target[i * 2] += voice->volume * (buffer[offset * 2] + blend * (buffer[next * 2] - buffer[offset * 2]));
        target[i * 2 + 1] += voice->volume * (buffer[offset * 2 + 1] + blend * (buffer[next * 2 + 1] - buffer[offset * 2 + 1]));
        voice->position += step;
    }
    return true;
}

static bool mix_clip(Voice *voice, float *target)
{
    const float *frames = voice->clip->frames;
    long count = voice->clip->frame_count;
    double step = voice->rate / OUTPUT_RATE;

    for (int i = 0; i < BLOCK_FRAMES; i++) {
        long whole = (long)voice->position;

        if (whole >= count)
            return false;

        long next = whole + 1 < count ? whole + 1 : whole;
        float blend = (float)(voice->position - (double)whole);

        target[i * 2] += voice->volume * (frames[whole * 2] + blend * (frames[next * 2] - frames[whole * 2]));
        target[i * 2 + 1] += voice->volume * (frames[whole * 2 + 1] + blend * (frames[next * 2 + 1] - frames[whole * 2 + 1]));
        voice->position += step;
    }
    return true;
}

static void mix_voice(Voice *voice, float *target, int index)
{
    if (voice->stop_requested) {
        end_voice(voice);
        return;
    }
    if (voice->state == VOICE_STARTING) {
        if (!begin_voice(voice)) {
            end_voice(voice);
            return;
        }
        voice->state = VOICE_PLAYING;
    }

    bool playing = voice->clip->stream ? mix_stream(voice, target, index) : mix_clip(voice, target);
    if (!playing)
        end_voice(voice);
}

static void mix_block(float *target)
{
    memset(target, 0, BLOCK_BYTES);
    if (paused)
        return;

    for (int index = 0; index < MAX_VOICES; index++) {
        if (voices[index].state != VOICE_FREE)
            mix_voice(&voices[index], target, index);
    }

    for (int i = 0; i < BLOCK_SAMPLES; i++) {
        if (target[i] > 1.0f) target[i] = 1.0f;
        if (target[i] < -1.0f) target[i] = -1.0f;
    }
}

/* ------------------------------------------------------------------ threads */

static void mixer_thread(void *argument)
{
    (void)argument;

    while (running) {
        if (ring_write - ring_read >= RING_BLOCKS) {
            usleep(1000);
            continue;
        }
        mix_block(ring[ring_write % RING_BLOCKS]);
        __asm__ volatile("lwsync");
        ring_write++;
    }
    sysThreadExit(0);
}

/* Does nothing but copy: mixing here would miss the 5.33 ms deadline. */
static void feeder_thread(void *argument)
{
    (void)argument;
    sys_event_t event;
    float *port = (float *)(u64)port_config.audioDataStart;

    while (running) {
        if (sysEventQueueReceive(notify_queue, &event, WAIT_MICROSECONDS) != 0)
            continue;

        u64 playing = *(vu64 *)(u64)port_config.readIndex;
        float *target = port + ((playing + 1) % PORT_BLOCKS) * BLOCK_SAMPLES;

        if (ring_write == ring_read) {
            memset(target, 0, BLOCK_BYTES);
            underruns++;
            continue;
        }
        memcpy(target, ring[ring_read % RING_BLOCKS], BLOCK_BYTES);
        __asm__ volatile("lwsync");
        ring_read++;
    }
    sysThreadExit(0);
}

int audio_open(void)
{
    audioPortParam parameters;

    if (audioInit() != 0)
        return -1;

    parameters.numChannels = AUDIO_PORT_2CH;
    parameters.numBlocks = AUDIO_BLOCK_8;
    parameters.attrib = 0;
    parameters.level = 1.0f;

    if (audioPortOpen(&parameters, &port_number) != 0)
        return -1;
    if (audioGetPortConfig(port_number, &port_config) != 0)
        return -1;
    if (audioCreateNotifyEventQueue(&notify_queue, &notify_key) != 0)
        return -1;
    if (audioSetNotifyEventQueue(notify_key) != 0)
        return -1;

    running = true;
    if (sysThreadCreate(&mixer_id, mixer_thread, NULL, MIXER_PRIORITY, THREAD_STACK, 0, "audio mixer") != 0)
        return -1;
    if (sysThreadCreate(&feeder_id, feeder_thread, NULL, FEEDER_PRIORITY, THREAD_STACK, 0, "audio feeder") != 0)
        return -1;
    if (audioPortStart(port_number) != 0)
        return -1;

    opened = true;
    return 0;
}

void audio_close(void)
{
    if (!opened)
        return;
    opened = false;
    running = false;
    /* Long enough for both threads to see the flag, and the port to play out silently. */
    usleep(50000);
    audioPortStop(port_number);
    audioPortClose(port_number);
    audioQuit();
}

/* ------------------------------------------------------------------ the game's side */

static unsigned char *read_whole_file(const char *path, size_t *size)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    *size = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *bytes = malloc(*size > 0 ? *size : 1);
    if (bytes != NULL && fread(bytes, 1, *size, file) != *size) {
        free(bytes);
        bytes = NULL;
    }
    fclose(file);
    return bytes;
}

AudioClip *audio_clip_load_wav(const char *path)
{
    size_t size;
    WavSound sound;
    unsigned char *bytes = read_whole_file(path, &size);

    if (bytes == NULL)
        return NULL;

    WavStatus status = wav_decode(bytes, size, &sound);
    free(bytes);
    if (status != WAV_OK) {
        printf("audio: %s is not a playable WAV (status %d)\n", path, (int)status);
        return NULL;
    }

    AudioClip *clip = calloc(1, sizeof(AudioClip));
    if (clip == NULL) {
        wav_free(&sound);
        return NULL;
    }
    clip->frames = sound.frames;
    clip->frame_count = sound.frame_count;
    clip->rate = sound.rate;
    return clip;
}

AudioClip *audio_clip_load_ogg(const char *path)
{
    AudioClip *clip = calloc(1, sizeof(AudioClip));

    if (clip == NULL)
        return NULL;
    clip->bytes = read_whole_file(path, &clip->size);
    if (clip->bytes == NULL) {
        free(clip);
        return NULL;
    }
    clip->stream = true;
    return clip;
}

/* A handle holds the voice's index and its generation, so an old handle cannot touch a new sound. */
static Voice *voice_of(AudioVoice handle)
{
    if (handle == AUDIO_NO_VOICE)
        return NULL;

    Voice *voice = &voices[handle & 0xFF];
    if (voice->generation != (unsigned)(handle >> 8) || voice->state == VOICE_FREE)
        return NULL;
    return voice;
}

AudioVoice audio_voice_start(AudioClip *clip, float volume, bool loop)
{
    if (!opened || clip == NULL)
        return AUDIO_NO_VOICE;

    for (int index = 0; index < MAX_VOICES; index++) {
        Voice *voice = &voices[index];

        if (voice->state != VOICE_FREE)
            continue;

        voice->generation = (voice->generation + 1) & 0xFFFFF;
        voice->clip = clip;
        voice->volume = volume;
        voice->loop = loop;
        voice->stop_requested = 0;
        __asm__ volatile("lwsync");
        voice->state = VOICE_STARTING;
        return (AudioVoice)((voice->generation << 8) | (unsigned)index);
    }
    return AUDIO_NO_VOICE;
}

void audio_voice_stop(AudioVoice handle)
{
    Voice *voice = voice_of(handle);

    if (voice != NULL)
        voice->stop_requested = 1;
}

bool audio_voice_is_playing(AudioVoice handle)
{
    Voice *voice = voice_of(handle);

    return voice != NULL && !voice->stop_requested;
}

void audio_stop_all(void)
{
    for (int index = 0; index < MAX_VOICES; index++) {
        if (voices[index].state != VOICE_FREE)
            voices[index].stop_requested = 1;
    }
}

void audio_set_paused(bool value)
{
    paused = value;
}

unsigned audio_underruns(void)
{
    return underruns;
}
