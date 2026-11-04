#ifndef PAD_EDGES_H
#define PAD_EDGES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Button masks for one frame: what is down, and what changed since the frame before. */
typedef struct {
    unsigned held;
    unsigned pressed;
    unsigned released;
} PadEdges;

PadEdges pad_edges_next(unsigned previous_held, unsigned now_held);

#ifdef __cplusplus
}
#endif

#endif
