#ifndef WAV_H
#define WAV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Uncompressed PCM decoded to interleaved stereo floats in -1..1. Mono plays on both sides. */
typedef struct {
    float *frames;
    long frame_count;
    int rate;
} WavSound;

typedef enum {
    WAV_OK,
    WAV_NOT_RIFF,
    WAV_UNSUPPORTED,
    WAV_TRUNCATED,
    WAV_NO_MEMORY
} WavStatus;

WavStatus wav_decode(const unsigned char *bytes, size_t size, WavSound *sound);
void wav_free(WavSound *sound);

#ifdef __cplusplus
}
#endif

#endif
