#ifndef VIDEO_H
#define VIDEO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A part of the game's screen, drawn into a rectangle of the TV picture in pixels. */
typedef struct {
    float x, y, width, height;
    float u0, v0, u1, v1;
} VideoQuad;

/* Display, surfaces and the program. Then one texture pair the size of the game's screen. */
int video_open(void);
int video_create_screen(int width, int height);
void video_close(void);
int video_width(void);
int video_height(void);

/* Per frame: copy the game's RGBA screen in, draw parts of it, flip at vsync. */
void video_upload_screen(const unsigned char *rgba, size_t pitch);
void video_draw(const VideoQuad *quads, int count);
void video_present(void);

/* Test tooling: writes the last drawn frame as a binary PPM. Slow on hardware. */
int video_dump_frame(const char *path);

#ifdef __cplusplus
}
#endif

#endif
