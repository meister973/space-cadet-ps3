#include "check.h"
#include "pad_edges.h"

static void a_button_held_now_but_not_before_is_pressed(void)
{
    PadEdges edges = pad_edges_next(0x0, 0x4);

    CHECK(edges.pressed == 0x4);
    CHECK(edges.released == 0x0);
    CHECK(edges.held == 0x4);
}

static void a_button_held_before_but_not_now_is_released(void)
{
    PadEdges edges = pad_edges_next(0x6, 0x2);

    CHECK(edges.pressed == 0x0);
    CHECK(edges.released == 0x4);
    CHECK(edges.held == 0x2);
}

static void a_button_held_in_both_frames_is_neither_pressed_nor_released(void)
{
    PadEdges edges = pad_edges_next(0x1, 0x1);

    CHECK(edges.pressed == 0x0);
    CHECK(edges.released == 0x0);
}

int main(void)
{
    RUN(a_button_held_now_but_not_before_is_pressed);
    RUN(a_button_held_before_but_not_now_is_released);
    RUN(a_button_held_in_both_frames_is_neither_pressed_nor_released);
    return 0;
}
