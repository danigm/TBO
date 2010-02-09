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

int
tbo_frame_point_inside (Frame *frame, int x, int y)
{
    if ((x >= frame->x) &&
            (x <= (frame->x + frame->width)) &&
            (y >= frame->y) &&
            (y <= (frame->y + frame->height)))
        return 1;
    else
        return 0;
}

void tbo_frame_draw_scaled (Frame *frame, cairo_t *cr, int width, int height)
{
    float scale_factor = 1.;
    float scale_factor2 = 1.;
    int RX, RY;

    scale_factor = (width-20) / (float)frame->width;
    scale_factor2 = (height-20) / (float)frame->height;

    scale_factor = scale_factor > scale_factor2 ? scale_factor2 : scale_factor;


    cairo_scale (cr, scale_factor, scale_factor);
    RX = frame->x;
    RY = frame->y;
    frame->x = (width/2.0) - (frame->width*scale_factor / 2.0);
    frame->x = frame->x / scale_factor;
    frame->y = (height/2.0) - (frame->height*scale_factor / 2.0);
    frame->y = frame->y / scale_factor;
    tbo_frame_draw(frame, cr);
    frame->x = RX;
    frame->y = RY;
}
