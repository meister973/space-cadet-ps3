#ifndef FILE_NAMES_H
#define FILE_NAMES_H

#ifdef __cplusplus
extern "C" {
#endif

/* The game asks for upper-case names, Windows copies keep any case, and the PS3 HDD is
 * case-sensitive. Returns the folder's spelling of wanted, or NULL when it is absent. */
const char *file_names_match(const char *const *names, int count, const char *wanted);

#ifdef __cplusplus
}
#endif

#endif
