#ifndef TEST_RUN_H
#define TEST_RUN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A test run in RPCS3. tools/run_rpcs3.ps1 writes dump_frames.txt and pad_script.txt beside the
 * game; without them every call below is a no-op, so a normal boot behaves the same. */
void test_run_open(const char *folder);

/* Call once per frame, before reading the pad. */
void test_run_next_frame(void);
long test_run_frame(void);
unsigned test_run_scripted_buttons(void);
bool test_run_wants_dump(void);

#ifdef __cplusplus
}
#endif

#endif
