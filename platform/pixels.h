#ifndef PIXELS_H
#define PIXELS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* The game's screen stores each pixel as bytes R, G, B, A. An A8R8G8B8 texture wants A, R, G, B. */
void pixels_copy_rgba_to_argb(const unsigned char *source, size_t source_pitch,
                              unsigned char *target, size_t target_pitch, int width, int height);

#ifdef __cplusplus
}
#endif

#endif
