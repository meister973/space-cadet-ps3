#include "pad_edges.h"

PadEdges pad_edges_next(unsigned previous_held, unsigned now_held)
{
    PadEdges edges;

    edges.held = now_held;
    edges.pressed = now_held & ~previous_held;
    edges.released = previous_held & ~now_held;
    return edges;
}
