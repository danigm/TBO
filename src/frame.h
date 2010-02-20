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
void tbo_frame_draw_scaled (Frame *frame, cairo_t *cr, int width, int height);
int tbo_frame_point_inside (Frame *frame, int x, int y);
int tbo_frame_point_inside_obj (tbo_object *obj, int x, int y);
void tbo_frame_add_obj (Frame *frame, tbo_object *obj);
void tbo_frame_get_obj_relative (tbo_object *obj, int *x, int *y, int *w, int *h);
float tbo_frame_get_scale_factor ();

#endif

