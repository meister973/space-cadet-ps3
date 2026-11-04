#include "check.h"
#include "screen_layout.h"

static void the_table_fills_the_height_of_a_720p_screen_and_is_centred(void)
{
    LayoutRect rect = layout_fit(1280, 720, 600, 416);

    CHECK_NEAR(rect.height, 720.0, 1e-3);
    CHECK_NEAR(rect.width, 600.0 * 720.0 / 416.0, 1e-3);
    CHECK_NEAR(rect.x, (1280.0 - 600.0 * 720.0 / 416.0) / 2.0, 1e-3);
    CHECK_NEAR(rect.y, 0.0, 1e-3);
}

static void on_a_narrow_screen_the_width_binds_and_the_table_is_centred_vertically(void)
{
    LayoutRect rect = layout_fit(600, 800, 600, 416);

    CHECK_NEAR(rect.width, 600.0, 1e-3);
    CHECK_NEAR(rect.height, 416.0, 1e-3);
    CHECK_NEAR(rect.x, 0.0, 1e-3);
    CHECK_NEAR(rect.y, 192.0, 1e-3);
}

static void on_a_480p_screen_the_height_still_binds(void)
{
    LayoutRect rect = layout_fit(720, 480, 600, 416);

    CHECK_NEAR(rect.height, 480.0, 1e-3);
    CHECK_NEAR(rect.x, (720.0 - 600.0 * 480.0 / 416.0) / 2.0, 1e-3);
}

int main(void)
{
    RUN(the_table_fills_the_height_of_a_720p_screen_and_is_centred);
    RUN(on_a_narrow_screen_the_width_binds_and_the_table_is_centred_vertically);
    RUN(on_a_480p_screen_the_height_still_binds);
    return 0;
}
