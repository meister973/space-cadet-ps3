#ifndef AUTOMATION_H
#define AUTOMATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Test runs in RPCS3: which frames to dump, and which pad buttons to hold from which frame. */
typedef struct {
    long frame;
    unsigned held;
} AutomationStep;

/* Both return how many entries were read, or -1 when the text is malformed. */
int automation_parse_frames(const char *text, long *frames, int capacity);

/* One step per line: a decimal frame number, then the held PAD_* mask in hexadecimal. */
int automation_parse_script(const char *text, AutomationStep *steps, int capacity);

/* The buttons of the last step at or before this frame. Steps are in frame order. */
unsigned automation_held_at(const AutomationStep *steps, int count, long frame);

#ifdef __cplusplus
}
#endif

#endif
