/* RIFF WAVE reader for the game's sound effects: 8-bit and 16-bit PCM, mono or stereo. */
#include "wav.h"

#include <stdlib.h>
#include <string.h>

#define FORMAT_PCM 1

typedef struct {
    int channels;
    int rate;
    int bits;
} PcmFormat;

typedef struct {
    const unsigned char *bytes;
    size_t size;
} Span;

static unsigned read_le16(const unsigned char *bytes) { return bytes[0] | (bytes[1] << 8); }

static unsigned long read_le32(const unsigned char *bytes)
{
    return (unsigned long)read_le16(bytes) | ((unsigned long)read_le16(bytes + 2) << 16);
}

/* Finds a chunk by tag. Its size is clamped to the bytes the file really has. */
static int find_chunk(const unsigned char *bytes, size_t size, const char *tag, Span *chunk)
{
    size_t position = 12;

    while (position + 8 <= size) {
        unsigned long length = read_le32(bytes + position + 4);
        size_t available = size - (position + 8);

        if (memcmp(bytes + position, tag, 4) == 0) {
            chunk->bytes = bytes + position + 8;
            chunk->size = length < available ? length : available;
            return 1;
        }
        position += 8 + length + (length & 1);
    }
    return 0;
}

static WavStatus read_format(const Span *chunk, PcmFormat *format)
{
    if (chunk->size < 16)
        return WAV_TRUNCATED;
    if (read_le16(chunk->bytes) != FORMAT_PCM)
        return WAV_UNSUPPORTED;

    format->channels = (int)read_le16(chunk->bytes + 2);
    format->rate = (int)read_le32(chunk->bytes + 4);
    format->bits = (int)read_le16(chunk->bytes + 14);

    if (format->channels < 1 || format->channels > 2 || (format->bits != 8 && format->bits != 16))
        return WAV_UNSUPPORTED;
    return WAV_OK;
}

static float sample_at(const unsigned char *bytes, int bits)
{
    if (bits == 8)
        return ((int)bytes[0] - 128) / 128.0f;
    return (short)read_le16(bytes) / 32768.0f;
}

static void convert_frames(const Span *data, const PcmFormat *format, WavSound *sound)
{
    int sample_bytes = format->bits / 8;
    int frame_bytes = sample_bytes * format->channels;
    int right_offset = format->channels > 1 ? sample_bytes : 0;

    for (long frame = 0; frame < sound->frame_count; frame++) {
        const unsigned char *source = data->bytes + frame * frame_bytes;

        sound->frames[frame * 2] = sample_at(source, format->bits);
        sound->frames[frame * 2 + 1] = sample_at(source + right_offset, format->bits);
    }
}

WavStatus wav_decode(const unsigned char *bytes, size_t size, WavSound *sound)
{
    Span format_chunk;
    Span data_chunk;
    PcmFormat format;

    memset(sound, 0, sizeof(*sound));
    if (size < 12 || memcmp(bytes, "RIFF", 4) != 0 || memcmp(bytes + 8, "WAVE", 4) != 0)
        return WAV_NOT_RIFF;
    if (!find_chunk(bytes, size, "fmt ", &format_chunk))
        return WAV_TRUNCATED;

    WavStatus status = read_format(&format_chunk, &format);
    if (status != WAV_OK)
        return status;
    if (!find_chunk(bytes, size, "data", &data_chunk))
        return WAV_TRUNCATED;

    sound->frame_count = (long)(data_chunk.size / (size_t)(format.bits / 8 * format.channels));
    sound->rate = format.rate;
    sound->frames = malloc((size_t)(sound->frame_count > 0 ? sound->frame_count : 1) * 2 * sizeof(float));
    if (sound->frames == NULL)
        return WAV_NO_MEMORY;

    convert_frames(&data_chunk, &format, sound);
    return WAV_OK;
}

void wav_free(WavSound *sound)
{
    free(sound->frames);
    sound->frames = NULL;
    sound->frame_count = 0;
}
