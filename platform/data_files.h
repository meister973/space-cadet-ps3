#ifndef DATA_FILES_H
#define DATA_FILES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* folder + name, with each part of name spelled as it is on disk. folder ends with a slash,
 * and name may hold sub-folders ("SOUND/TABA1.ogg"). A part with no match is kept as given. */
void data_files_path(const char *folder, const char *name, char *path, size_t size);

#ifdef __cplusplus
}
#endif

#endif
