#include "screen_layout.h"

LayoutRect layout_fit(int screen_width, int screen_height, int source_width, int source_height)
{
    float scale_x = (float)screen_width / (float)source_width;
    float scale_y = (float)screen_height / (float)source_height;
    float scale = scale_x < scale_y ? scale_x : scale_y;
    LayoutRect rect;

    rect.width = source_width * scale;
    rect.height = source_height * scale;
    rect.x = (screen_width - rect.width) / 2.0f;
    rect.y = (screen_height - rect.height) / 2.0f;
    return rect;
}
