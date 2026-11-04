#include <string.h>

#include "check.h"
#include "file_names.h"

static const char *const folder[] = { "pinball.dat", "Sound1.wav", "PINBALL.MID" };

static void a_name_is_found_whatever_its_case_on_disk(void)
{
    CHECK(strcmp(file_names_match(folder, 3, "PINBALL.DAT"), "pinball.dat") == 0);
    CHECK(strcmp(file_names_match(folder, 3, "SOUND1.WAV"), "Sound1.wav") == 0);
}

static void an_exact_name_is_returned_as_it_is(void)
{
    CHECK(strcmp(file_names_match(folder, 3, "PINBALL.MID"), "PINBALL.MID") == 0);
}

static void a_name_not_in_the_folder_is_not_found(void)
{
    CHECK(file_names_match(folder, 3, "CADET.DAT") == NULL);
}

static void a_longer_name_with_the_same_start_is_not_a_match(void)
{
    CHECK(file_names_match(folder, 3, "PINBALL.DATA") == NULL);
}

int main(void)
{
    RUN(a_name_is_found_whatever_its_case_on_disk);
    RUN(an_exact_name_is_returned_as_it_is);
    RUN(a_name_not_in_the_folder_is_not_found);
    RUN(a_longer_name_with_the_same_start_is_not_a_match);
    return 0;
}
