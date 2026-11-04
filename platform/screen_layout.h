#ifndef SCREEN_LAYOUT_H
#define SCREEN_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x, y, width, height;
} LayoutRect;

/* The largest rectangle with the source's aspect that fits the screen, centred. */
LayoutRect layout_fit(int screen_width, int screen_height, int source_width, int source_height);

#ifdef __cplusplus
}
#endif

#endif
