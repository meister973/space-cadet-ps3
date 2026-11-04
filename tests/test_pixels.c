#include <string.h>

#include "check.h"
#include "pixels.h"

static void each_pixel_moves_alpha_to_the_front(void)
{
    const unsigned char screen[] = { 10, 20, 30, 255 };
    unsigned char texture[4] = { 0 };

    pixels_copy_rgba_to_argb(screen, 4, texture, 4, 1, 1);

    CHECK(texture[0] == 255);
    CHECK(texture[1] == 10);
    CHECK(texture[2] == 20);
    CHECK(texture[3] == 30);
}

static void row_padding_on_both_sides_is_skipped_and_left_untouched(void)
{
    /* Two rows of one pixel. The screen has 4 bytes of padding per row, the texture 8. */
    const unsigned char screen[] = { 1, 2, 3, 4, 99, 99, 99, 99, 5, 6, 7, 8, 99, 99, 99, 99 };
    unsigned char texture[24];

    memset(texture, 0xEE, sizeof(texture));
    pixels_copy_rgba_to_argb(screen, 8, texture, 12, 1, 2);

    CHECK(memcmp(texture, (const unsigned char[]){ 4, 1, 2, 3 }, 4) == 0);
    CHECK(texture[4] == 0xEE && texture[11] == 0xEE);
    CHECK(memcmp(texture + 12, (const unsigned char[]){ 8, 5, 6, 7 }, 4) == 0);
    CHECK(texture[16] == 0xEE);
}

int main(void)
{
    RUN(each_pixel_moves_alpha_to_the_front);
    RUN(row_padding_on_both_sides_is_skipped_and_left_untouched);
    return 0;
}
