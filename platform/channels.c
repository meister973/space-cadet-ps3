#include "channels.h"

int channels_pick(const int *start_times, const bool *busy, int count)
{
    int oldest = 0;

    for (int channel = 0; channel < count; channel++) {
        if (!busy[channel])
            return channel;
        if (start_times[channel] < start_times[oldest])
            oldest = channel;
    }
    return oldest;
}
