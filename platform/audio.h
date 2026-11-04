#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AudioClip AudioClip;

/* A voice handle stays safe after its voice ends: the calls below then do nothing. */
typedef int AudioVoice;
#define AUDIO_NO_VOICE (-1)

int audio_open(void);
void audio_close(void);

/* A WAV sound effect, decoded once. An Ogg file, decoded while it plays. NULL when unreadable.
 * Clips live until the process exits. */
AudioClip *audio_clip_load_wav(const char *path);
AudioClip *audio_clip_load_ogg(const char *path);

AudioVoice audio_voice_start(AudioClip *clip, float volume, bool loop);
void audio_voice_stop(AudioVoice voice);
bool audio_voice_is_playing(AudioVoice voice);
void audio_stop_all(void);
void audio_set_paused(bool paused);

/* Blocks played as silence because the mixer fell behind. Each one is an audible click. */
unsigned audio_underruns(void);

#ifdef __cplusplus
}
#endif

#endif
