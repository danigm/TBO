#include <gtk/gtk.h>
#include <math.h>
#include <cairo.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "frame.h"
#include "tbo-types.h"
#include "tbo-object.h"

static int BASE_X = 0;
static int BASE_Y = 0;
static float SCALE_FACTOR = 0;
static Color BASE_COLOR = {1, 1, 1};

void
tbo_frame_set_scale_factor (Frame *frame, int width, int height)
{
    float scale_factor = 1.;
    float scale_factor2 = 1.;

    scale_factor = (width-20) / (float)frame->width;
    scale_factor2 = (height-20) / (float)frame->height;

    scale_factor = scale_factor > scale_factor2 ? scale_factor2 : scale_factor;

    SCALE_FACTOR = scale_factor;
}

int
tbo_frame_get_x_centered (Frame *frame, int width)
{
    int x;
    x = (width/2.0) - (frame->width*SCALE_FACTOR / 2.0);
    x = x / SCALE_FACTOR;
    return x;
}

int
tbo_frame_get_y_centered (Frame *frame, int height)
{
    int y;
    y = (height/2.0) - (frame->height*SCALE_FACTOR / 2.0);
    y = y / SCALE_FACTOR;
    return y;
}

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
    new_frame->border = TRUE;
    new_frame->color = malloc (sizeof (Color));
    new_frame->color->r = BASE_COLOR.r;
    new_frame->color->g = BASE_COLOR.g;
    new_frame->color->b = BASE_COLOR.b;

    return new_frame;
}

static void
free_objects (gpointer data,
              gpointer user_data)
{
    tbo_object *obj = (tbo_object *)data;
    obj->free (obj);
}

void
tbo_frame_free (Frame *frame)
{
    g_list_foreach (g_list_first (frame->objects), free_objects, NULL);
    g_list_free (frame->objects);
    free (frame->color);
    free (frame);
}

static void
draw_objects (gpointer data,
              gpointer user_data)
{
    void **pdata = user_data;
    tbo_object *obj = (tbo_object *)data;
    cairo_t *cr = (cairo_t *)pdata[0];
    Frame *frame = (Frame *)pdata[1];
    obj->draw (obj, frame, cr);
}

void
tbo_frame_draw_complete (Frame *frame, cairo_t *cr,
        float fill_r, float fill_g, float fill_b,
        float border_r, float border_g, float border_b,
        int line_width)
{
    cairo_set_source_rgb(cr, fill_r, fill_g, fill_b);
    cairo_rectangle(cr, frame->x, frame->y,
            frame->width, frame->height);
    cairo_fill(cr);

    cairo_set_line_width (cr, line_width);
    if (frame->border)
    {
        cairo_set_source_rgb(cr, border_r, border_g, border_b);
        cairo_rectangle (cr, frame->x, frame->y,
                frame->width, frame->height);
        cairo_stroke (cr);
    }

    void **crframe = malloc (sizeof(void*)*2);
    crframe[0] = (void*)cr;
    crframe[1] = (void*)frame;

    g_list_foreach (g_list_first (frame->objects), draw_objects, crframe);

    free (crframe);
}

void
tbo_frame_draw (Frame *frame, cairo_t *cr)
{
    Color border = {0, 0, 0};
    Color *fill = frame->color;
    tbo_frame_draw_complete (frame, cr,
            fill->r, fill->g, fill->b,
            border.r, border.g, border.b,
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

int
tbo_frame_point_inside_obj (tbo_object *obj, int x, int y)
{
    int ox, oy, ow, oh;

    tbo_frame_get_obj_relative (obj, &ox, &oy, &ow, &oh);
    int xnew1 = ox + (ow * cos (obj->angle));
    int ynew1 = oy + (ow * sin (obj->angle));
    int xnew2 = ox + (-oh * sin (obj->angle));
    int ynew2 = oy + (oh * cos (obj->angle));
    int xnew3 = ox + (ow * cos(obj->angle) - oh * sin(obj->angle));
    int ynew3 = oy + (oh * cos(obj->angle) + ow * sin(obj->angle));

    int xmax, ymax, xmin, ymin;

    xmax = ox;
    if (xnew1 > xmax)
        xmax = xnew1;
    if (xnew2 > xmax)
        xmax = xnew2;
    if (xnew3 > xmax)
        xmax = xnew3;
    ymax = oy;
    if (ynew1 > ymax)
        ymax = ynew1;
    if (ynew2 > ymax)
        ymax = ynew2;
    if (ynew3 > ymax)
        ymax = ynew3;

    xmin = ox;
    if (xnew1 < xmin)
        xmin = xnew1;
    if (xnew2 < xmin)
        xmin = xnew2;
    if (xnew3 < xmin)
        xmin = xnew3;
    ymin = oy;
    if (ynew1 < ymin)
        ymin = ynew1;
    if (ynew2 < ymin)
        ymin = ynew2;
    if (ynew3 < ymin)
        ymin = ynew3;

    if ((x >= xmin) &&
            (x <= xmax) &&
            (y >= ymin) &&
            (y <= ymax))
        return 1;
    else
        return 0;
}

void
tbo_frame_get_obj_relative (tbo_object *obj, int *x, int *y, int *w, int *h)
{
    *x = (BASE_X + obj->x) * SCALE_FACTOR;
    *w = obj->width * SCALE_FACTOR;
    *y = (BASE_Y + obj->y) * SCALE_FACTOR;
    *h = obj->height * SCALE_FACTOR;
}

int
tbo_frame_get_base_x (int x)
{
    return (x / SCALE_FACTOR) - BASE_X;
}

int
tbo_frame_get_base_y (int y)
{
    return (y / SCALE_FACTOR) - BASE_Y;
}

void tbo_frame_draw_scaled (Frame *frame, cairo_t *cr, int width, int height)
{
    int RX, RY;
    tbo_frame_set_scale_factor (frame, width, height);

    cairo_scale (cr, SCALE_FACTOR, SCALE_FACTOR);
    RX = frame->x;
    RY = frame->y;
    frame->x = tbo_frame_get_x_centered (frame, width);
    frame->y = tbo_frame_get_y_centered (frame, height);
    BASE_X = frame->x;
    BASE_Y = frame->y;
    tbo_frame_draw (frame, cr);
    cairo_scale (cr, 1/SCALE_FACTOR, 1/SCALE_FACTOR);
    frame->x = RX;
    frame->y = RY;
}

void
tbo_frame_add_obj (Frame *frame, tbo_object *obj)
{
    frame->objects = g_list_append (frame->objects, obj);
}

float
tbo_frame_get_scale_factor ()
{
    return SCALE_FACTOR;
}

void
tbo_frame_del_obj (Frame *frame, tbo_object *obj)
{
    frame->objects = g_list_remove (g_list_first (frame->objects), obj);
    obj->free (obj);
}

void
tbo_frame_set_color (Frame *frame, GdkColor *color)
{
    frame->color->r = color->red / 65535.0;
    frame->color->g = color->green / 65535.0;
    frame->color->b = color->blue / 65535.0;
    BASE_COLOR.r = frame->color->r;
    BASE_COLOR.g = frame->color->g;
    BASE_COLOR.b = frame->color->b;
}

void
tbo_frame_save (Frame *frame, FILE *file)
{
    char buffer[255];
    GList *o;

    snprintf (buffer, 255, "  <frame x=\"%d\" y=\"%d\" width=\"%d\" "
                           "height=\"%d\" border=\"%d\" "
                           "r=\"%f\" g=\"%f\" b=\"%f\">\n",
                            frame->x, frame->y, frame->width,
                            frame->height, frame->border,
                            frame->color->r, frame->color->g, frame->color->b);
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    for (o=g_list_first (frame->objects); o; o = g_list_next(o))
    {
        tbo_object_save ((tbo_object *) o->data, file);
    }

    snprintf (buffer, 255, "  </frame>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
}

Frame *
tbo_frame_clone (Frame *frame)
{
    GList *o;
    tbo_object *cur_object;
    Frame *newframe = tbo_frame_new (frame->x, frame->y,
                                     frame->width, frame->height);

    for (o=g_list_first (frame->objects); o; o = g_list_next(o))
    {
        cur_object = (tbo_object *) o->data;
        tbo_frame_add_obj (newframe, cur_object->clone (cur_object));
    }
    newframe->border = frame->border;
    newframe->color->r = frame->color->r;
    newframe->color->g = frame->color->g;
    newframe->color->b = frame->color->b;

    return newframe;
}
