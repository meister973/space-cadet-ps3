#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "automation.h"
#include "system.h"
#include "test_run.h"

#define MAX_DUMPS 32
#define MAX_STEPS 256
#define MAX_FILE_BYTES 16384

static long dump_frames[MAX_DUMPS];
static int dump_count;
static AutomationStep steps[MAX_STEPS];
static int step_count;
static long frame = -1;

static char *read_text(const char *folder, const char *name)
{
    char path[256];
    snprintf(path, sizeof(path), "%s%s", folder, name);

    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return NULL;

    char *text = calloc(1, MAX_FILE_BYTES + 1);
    if (text != NULL)
        fread(text, 1, MAX_FILE_BYTES, file);
    fclose(file);
    return text;
}

void test_run_open(const char *folder)
{
    char *frames_text = read_text(folder, "dump_frames.txt");
    char *script_text = read_text(folder, "pad_script.txt");

    if (frames_text != NULL) {
        dump_count = automation_parse_frames(frames_text, dump_frames, MAX_DUMPS);
        system_log("test run: %d frame dumps\n", dump_count);
    }
    if (script_text != NULL) {
        step_count = automation_parse_script(script_text, steps, MAX_STEPS);
        system_log("test run: %d scripted pad steps\n", step_count);
    }
    if (dump_count < 0 || step_count < 0)
        system_log("test run: a file in %s is malformed and is ignored\n", folder);
    if (dump_count < 0) dump_count = 0;
    if (step_count < 0) step_count = 0;

    free(frames_text);
    free(script_text);
}

void test_run_next_frame(void) { frame++; }
long test_run_frame(void) { return frame; }

unsigned test_run_scripted_buttons(void)
{
    return automation_held_at(steps, step_count, frame);
}

bool test_run_wants_dump(void)
{
    for (int i = 0; i < dump_count; i++) {
        if (dump_frames[i] == frame)
            return true;
    }
    return false;
}
