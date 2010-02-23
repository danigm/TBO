#include <stdlib.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "textobj.h"
#include "text-tool.h"
#include "tbo-types.h"

typedef struct
{
    double r;
    double g;
    double b;
} Color;

typedef struct
{
    GString *text;
    Color *font_color;
    gchar *font_name;
} text_data;

text_data *
text_data_new (const char *text, char *font_name, double r, double g, double b)
{
    text_data *txt = malloc (sizeof (text_data));
    txt->text = g_string_new (text);
    Color *color = malloc (sizeof (Color));
    color->r = r;
    color->g = g;
    color->b = b;
    txt->font_color = color;
    txt->font_name = font_name;

    return txt;
}

void
text_data_free (text_data *txt)
{
    free (txt->font_color);
    g_string_free (txt->text, TRUE);
}

TextObj *
tbo_text_new ()
{
    TextObj *text;
    text = malloc (sizeof(TextObj));
    text->data = text_data_new ("text", "Sans", 0, 0, 0);
    text->free = tbo_text_free;
    text->draw = tbo_text_draw;
    return text;
}

TextObj *
tbo_text_new_width_params (int x,
                           int y,
                           int width,
                           int height,
                           const char *text,
                           char *font_name,
                           double r, double g, double b)
{
    TextObj *textobj;
    textobj = tbo_text_new ();
    textobj->x = x;
    textobj->y = y;
    textobj->width = width;
    textobj->height = height;
    textobj->data = text_data_new (text, font_name, r, g, b);
    return textobj;
}

void
tbo_text_free (TextObj *self)
{
    text_data_free (self->data);
    free (self);
}

void
tbo_text_draw (TextObj *self, Frame *frame, cairo_t *cr)
{
    cairo_text_extents_t extents;
    text_data *data = self->data;
    gchar *text = data->text->str;

    cairo_set_source_rgb(cr, data->font_color->r, data->font_color->g, data->font_color->b);
    cairo_select_font_face (cr, data->font_name, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 90.0);

    cairo_text_extents (cr, text, &extents);
    int w = extents.width;
    int h = extents.height;


    if (!self->width) self->width = w;
    if (!self->height) self->height = h;

    float factorw = (float)self->width / (float)w;
    float factorh = (float)self->height / (float)h;

    cairo_rectangle(cr, frame->x+2, frame->y+2, frame->width-4, frame->height-4);
    cairo_clip (cr);
    cairo_translate (cr, frame->x+self->x, frame->y+self->y);
    cairo_rotate (cr, self->angle);
    cairo_scale (cr, factorw, factorh);

    cairo_move_to (cr, 0, h);
    cairo_show_text (cr, text);

    cairo_scale (cr, 1/factorw, 1/factorh);
    cairo_rotate (cr, -self->angle);
    cairo_translate (cr, -(frame->x+self->x), -(frame->y+self->y));
    cairo_reset_clip (cr);
}
