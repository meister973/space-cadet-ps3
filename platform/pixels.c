#include "pixels.h"

void pixels_copy_rgba_to_argb(const unsigned char *source, size_t source_pitch,
                              unsigned char *target, size_t target_pitch, int width, int height)
{
    for (int y = 0; y < height; y++) {
        const unsigned char *from = source + (size_t)y * source_pitch;
        unsigned char *to = target + (size_t)y * target_pitch;

        for (int x = 0; x < width; x++, from += 4, to += 4) {
            to[0] = from[3];
            to[1] = from[0];
            to[2] = from[1];
            to[3] = from[2];
        }
    }
}
