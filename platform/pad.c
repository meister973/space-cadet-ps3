#include <string.h>

#include <io/pad.h>

#include "pad.h"

#define PORT_COUNT 7

static padData last_data[PORT_COUNT];
static unsigned previous_held;

/* ioPadGetData reports len 0 when nothing changed since the last call, so the last real
 * state is kept, or a held flipper would look released. */
static const padData *current_data(int port, int connected)
{
    padData fresh;

    if (!connected) {
        memset(&last_data[port], 0, sizeof(padData));
        return &last_data[port];
    }
    if (ioPadGetData(port, &fresh) == 0 && fresh.len > 0)
        last_data[port] = fresh;
    return &last_data[port];
}

static unsigned buttons_of(const padData *data)
{
    unsigned held = 0;

    if (data->BTN_CROSS) held |= PAD_CROSS;
    if (data->BTN_CIRCLE) held |= PAD_CIRCLE;
    if (data->BTN_SQUARE) held |= PAD_SQUARE;
    if (data->BTN_TRIANGLE) held |= PAD_TRIANGLE;
    if (data->BTN_L1) held |= PAD_L1;
    if (data->BTN_R1) held |= PAD_R1;
    if (data->BTN_L2) held |= PAD_L2;
    if (data->BTN_R2) held |= PAD_R2;
    if (data->BTN_UP) held |= PAD_UP;
    if (data->BTN_DOWN) held |= PAD_DOWN;
    if (data->BTN_LEFT) held |= PAD_LEFT;
    if (data->BTN_RIGHT) held |= PAD_RIGHT;
    if (data->BTN_START) held |= PAD_START;
    if (data->BTN_SELECT) held |= PAD_SELECT;
    return held;
}

int pad_open(void)
{
    memset(last_data, 0, sizeof(last_data));
    return ioPadInit(PORT_COUNT);
}

/* Checked every frame: a pad can sleep and come back on another port. */
PadEdges pad_read(unsigned extra_held)
{
    padInfo info;
    unsigned held = extra_held;
    int have_info = ioPadGetInfo(&info) == 0;

    for (int port = 0; port < PORT_COUNT; port++)
        held |= buttons_of(current_data(port, have_info && info.status[port] != 0));

    PadEdges edges = pad_edges_next(previous_held, held);
    previous_held = held;
    return edges;
}
