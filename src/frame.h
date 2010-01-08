#ifndef __TBO_FRAME__
#define __TBO_FRAME__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-types.h"

Frame *tbo_frame_new (int x, int y, int witdh, int heigth);
void tbo_frame_free (Frame *frame);

void tbo_frame_draw_complete (Frame *frame, cairo_t *cr,
        float fill_r, float fill_g, float fill_b,
        float border_r, float border_g, float border_b,
        int line_width);

void tbo_frame_draw (Frame *frame, cairo_t *cr);

#endif

