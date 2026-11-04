#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The folder the game data goes in, ending with a slash. */
#define SYSTEM_DATA_FOLDER "/dev_hdd0/game/PNBL00001/USRDIR/Data/"
#define SYSTEM_LOG_PATH    "/dev_hdd0/tmp/spacecadet_log.txt"

void system_open(void);

/* True once the user picked Quit Game in the PS button menu. */
bool system_exit_requested(void);

/* Writes to the TTY, which RPCS3 shows, and to SYSTEM_LOG_PATH, which survives a hardware crash. */
void system_log(const char *format, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#endif
