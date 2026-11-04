#include "automation.h"

#include <ctype.h>
#include <stdlib.h>

static const char *skip_space(const char *text)
{
    while (*text != '\0' && isspace((unsigned char)*text))
        text++;
    return text;
}

int automation_parse_frames(const char *text, long *frames, int capacity)
{
    int count = 0;

    for (text = skip_space(text); *text != '\0' && count < capacity; text = skip_space(text)) {
        char *end;
        long frame = strtol(text, &end, 10);

        if (end == text)
            return -1;
        frames[count++] = frame;
        text = end;
    }
    return count;
}

/* Reads "<frame> <hex mask>" and leaves the cursor after it, or returns 0 when malformed. */
static int parse_step(const char **cursor, AutomationStep *step)
{
    char *end;
    const char *text = *cursor;

    step->frame = strtol(text, &end, 10);
    if (end == text || (*end != ' ' && *end != '\t'))
        return 0;

    text = end;
    step->held = (unsigned)strtoul(text, &end, 16);
    if (end == text)
        return 0;

    *cursor = end;
    return 1;
}

int automation_parse_script(const char *text, AutomationStep *steps, int capacity)
{
    int count = 0;

    for (text = skip_space(text); *text != '\0' && count < capacity; text = skip_space(text)) {
        if (!parse_step(&text, &steps[count]))
            return -1;
        count++;
    }
    return count;
}

unsigned automation_held_at(const AutomationStep *steps, int count, long frame)
{
    unsigned held = 0;

    for (int i = 0; i < count && steps[i].frame <= frame; i++)
        held = steps[i].held;
    return held;
}
