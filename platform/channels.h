#ifndef CHANNELS_H
#define CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/* The channel a new sound plays on: the first idle one, or else the one whose sound started first. */
int channels_pick(const int *start_times, const bool *busy, int count);

#ifdef __cplusplus
}
#endif

#endif
