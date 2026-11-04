#include "file_names.h"

#include <ctype.h>
#include <stddef.h>

static int same_ignoring_case(const char *left, const char *right)
{
    for (; *left != '\0' && *right != '\0'; left++, right++) {
        if (toupper((unsigned char)*left) != toupper((unsigned char)*right))
            return 0;
    }
    return *left == *right;
}

const char *file_names_match(const char *const *names, int count, const char *wanted)
{
    for (int i = 0; i < count; i++) {
        if (same_ignoring_case(names[i], wanted))
            return names[i];
    }
    return NULL;
}
