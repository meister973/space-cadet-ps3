#include "automation.h"
#include "check.h"

static void a_frame_list_is_numbers_separated_by_spaces_or_lines(void)
{
    long frames[4];

    CHECK(automation_parse_frames("600 1200\n2400\n", frames, 4) == 3);
    CHECK(frames[0] == 600 && frames[1] == 1200 && frames[2] == 2400);
}

static void a_frame_list_longer_than_the_room_keeps_the_first_frames(void)
{
    long frames[2];

    CHECK(automation_parse_frames("1 2 3", frames, 2) == 2);
    CHECK(frames[1] == 2);
}

static void a_frame_list_with_a_word_in_it_is_malformed(void)
{
    long frames[4];

    CHECK(automation_parse_frames("600 soon", frames, 4) == -1);
}

static void a_script_step_holds_its_buttons_until_the_next_step(void)
{
    AutomationStep steps[4];
    int count = automation_parse_script("300 1\n360 0\n", steps, 4);

    CHECK(count == 2);
    CHECK(automation_held_at(steps, count, 299) == 0);
    CHECK(automation_held_at(steps, count, 300) == 1);
    CHECK(automation_held_at(steps, count, 359) == 1);
    CHECK(automation_held_at(steps, count, 360) == 0);
}

static void script_buttons_are_read_as_hexadecimal(void)
{
    AutomationStep steps[1];

    CHECK(automation_parse_script("10 30", steps, 1) == 1);
    CHECK(steps[0].held == 0x30);
}

static void a_script_line_without_buttons_is_malformed(void)
{
    AutomationStep steps[2];

    CHECK(automation_parse_script("10", steps, 2) == -1);
}

int main(void)
{
    RUN(a_frame_list_is_numbers_separated_by_spaces_or_lines);
    RUN(a_frame_list_longer_than_the_room_keeps_the_first_frames);
    RUN(a_frame_list_with_a_word_in_it_is_malformed);
    RUN(a_script_step_holds_its_buttons_until_the_next_step);
    RUN(script_buttons_are_read_as_hexadecimal);
    RUN(a_script_line_without_buttons_is_malformed);
    return 0;
}
