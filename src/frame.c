#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include "frame.h"

Frame *
tbo_frame_new (int x, int y,
        int width, int height)
{
    Frame *new_frame;

    new_frame = malloc (sizeof(Frame));
    new_frame->objects = NULL;

    new_frame->x = x;
    new_frame->y = y;
    new_frame->width = width;
    new_frame->height = height;

    return new_frame;
}

void 
tbo_frame_free (Frame *frame)
{
    // TODO free all frame objects
    g_list_free (frame->objects);
    free (frame);
}

void
tbo_frame_draw_complete (Frame *frame, cairo_t *cr,
        float fill_r, float fill_g, float fill_b,
        float border_r, float border_g, float border_b,
        int line_width)
{
    cairo_set_line_width (cr, line_width);
    cairo_set_source_rgb(cr, fill_r, fill_g, fill_b);
    cairo_rectangle(cr, frame->x, frame->y,
            frame->width, frame->height);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, border_r, border_g, border_b);
    cairo_rectangle (cr, frame->x, frame->y,
            frame->width, frame->height);
    cairo_stroke (cr);
}

void tbo_frame_draw (Frame *frame, cairo_t *cr)
{
    tbo_frame_draw_complete (frame, cr,
            1, 1, 1,
            0, 0, 0,
            4);
}

