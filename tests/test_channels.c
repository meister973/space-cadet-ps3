#include <stdbool.h>

#include "channels.h"
#include "check.h"

static void a_new_sound_takes_the_first_idle_channel(void)
{
    const int start_times[] = { 50, 10, 30 };
    const bool busy[] = { true, false, false };

    CHECK(channels_pick(start_times, busy, 3) == 1);
}

static void when_every_channel_is_busy_the_oldest_sound_is_replaced(void)
{
    const int start_times[] = { 50, 10, 30 };
    const bool busy[] = { true, true, true };

    CHECK(channels_pick(start_times, busy, 3) == 1);
}

static void between_equally_old_sounds_the_lowest_channel_is_replaced(void)
{
    const int start_times[] = { 20, 20 };
    const bool busy[] = { true, true };

    CHECK(channels_pick(start_times, busy, 2) == 0);
}

int main(void)
{
    RUN(a_new_sound_takes_the_first_idle_channel);
    RUN(when_every_channel_is_busy_the_oldest_sound_is_replaced);
    RUN(between_equally_old_sounds_the_lowest_channel_is_replaced);
    return 0;
}
