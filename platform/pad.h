#ifndef PAD_H
#define PAD_H

#include "pad_edges.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PAD_CROSS    = 1u << 0,
    PAD_CIRCLE   = 1u << 1,
    PAD_SQUARE   = 1u << 2,
    PAD_TRIANGLE = 1u << 3,
    PAD_L1       = 1u << 4,
    PAD_R1       = 1u << 5,
    PAD_L2       = 1u << 6,
    PAD_R2       = 1u << 7,
    PAD_UP       = 1u << 8,
    PAD_DOWN     = 1u << 9,
    PAD_LEFT     = 1u << 10,
    PAD_RIGHT    = 1u << 11,
    PAD_START    = 1u << 12,
    PAD_SELECT   = 1u << 13
};

int pad_open(void);

/* Every connected pad counts, so any controller can play. Call once per frame.
 * extra_held adds buttons that are not on any pad, such as a test script's. */
PadEdges pad_read(unsigned extra_held);

#ifdef __cplusplus
}
#endif

#endif
