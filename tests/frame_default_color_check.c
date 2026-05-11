#include <math.h>

#include "frame.h"

int
main (void)
{
    Frame *first;
    Frame *second;
    GdkRGBA color;

    first = tbo_frame_new (0, 0, 100, 100);
    second = tbo_frame_new (0, 0, 100, 100);

    tbo_frame_set_color_rgb (first, 0.2, 0.4, 0.6);
    tbo_frame_get_color (second, &color);

    if (fabs (color.red - 1.0) > 1e-9 ||
        fabs (color.green - 1.0) > 1e-9 ||
        fabs (color.blue - 1.0) > 1e-9)
        return 2;

    tbo_frame_free (first);
    tbo_frame_free (second);
    return 0;
}
