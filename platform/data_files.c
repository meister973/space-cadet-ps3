#include <stdio.h>
#include <string.h>

#include <sys/file.h>

#include "data_files.h"
#include "file_names.h"

#define MAX_ENTRIES 512
#define NAME_BYTES  256

static char entry_names[MAX_ENTRIES][NAME_BYTES];
static const char *entries[MAX_ENTRIES];

static int list_folder(const char *folder)
{
    s32 directory;
    sysFSDirent entry;
    u64 read_size;
    int count = 0;

    if (sysLv2FsOpenDir(folder, &directory) != 0)
        return 0;

    while (count < MAX_ENTRIES && sysLv2FsReadDir(directory, &entry, &read_size) == 0 && read_size != 0) {
        size_t length = strnlen(entry.d_name, sizeof(entry.d_name));

        /* No file the game asks for has a name this long. */
        if (length >= NAME_BYTES)
            continue;
        memcpy(entry_names[count], entry.d_name, length);
        entry_names[count][length] = '\0';
        entries[count] = entry_names[count];
        count++;
    }
    sysLv2FsCloseDir(directory);
    return count;
}

/* Appends one part of the name to path, as spelled on disk when the folder holds it. */
static void append_part(char *path, size_t size, const char *part, size_t length)
{
    char wanted[NAME_BYTES];
    snprintf(wanted, sizeof(wanted), "%.*s", (int)length, part);

    const char *found = file_names_match(entries, list_folder(path), wanted);
    strncat(path, found != NULL ? found : wanted, size - strlen(path) - 1);
}

void data_files_path(const char *folder, const char *name, char *path, size_t size)
{
    snprintf(path, size, "%s", folder);

    while (*name != '\0') {
        const char *end = strchr(name, '/');
        size_t length = end != NULL ? (size_t)(end - name) : strlen(name);

        append_part(path, size, name, length);
        if (end == NULL)
            return;
        strncat(path, "/", size - strlen(path) - 1);
        name = end + 1;
    }
}
