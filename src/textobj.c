#include <stdlib.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "textobj.h"
#include "text-tool.h"
#include "tbo-types.h"

typedef struct
{
    GString *text;
    PangoFontDescription *description;
    Color *font_color;
} text_data;

text_data *
text_data_new (const char *text, char *font, double r, double g, double b)
{
    text_data *txt = malloc (sizeof (text_data));
    txt->text = g_string_new (text);
    Color *color = malloc (sizeof (Color));
    color->r = r;
    color->g = g;
    color->b = b;
    txt->font_color = color;
    txt->description = pango_font_description_from_string (font);

    return txt;
}

void
text_data_free (text_data *txt)
{
    free (txt->font_color);
    g_string_free (txt->text, TRUE);
    pango_font_description_free (txt->description);
    free (txt);
}

void
tbo_text_change_font (TextObj *self, char *font)
{
    text_data *txt = (text_data*)self->data;
    pango_font_description_free (txt->description);
    txt->description = pango_font_description_from_string (font);
}

void
tbo_text_change_color (TextObj *self, double r, double g, double b)
{
    text_data *txt = (text_data*)self->data;
    txt->font_color->r = r;
    txt->font_color->g = g;
    txt->font_color->b = b;
}

TextObj *
tbo_text_new ()
{
    TextObj *text;
    text = malloc (sizeof(TextObj));
    text->data = text_data_new ("text", "Sans Normal 27", 0, 0, 0);
    text->free = tbo_text_free;
    text->draw = tbo_text_draw;
    text->type = TEXTOBJ;
    text->flipv = FALSE;
    text->fliph = FALSE;
    return text;
}

TextObj *
tbo_text_new_width_params (int x,
                           int y,
                           int width,
                           int height,
                           const char *text,
                           char *font,
                           double r, double g, double b)
{
    TextObj *textobj;
    textobj = tbo_text_new ();
    textobj->x = x;
    textobj->y = y;
    textobj->width = width;
    textobj->height = height;
    textobj->data = text_data_new (text, font, r, g, b);
    textobj->type = TEXTOBJ;
    textobj->flipv = FALSE;
    textobj->fliph = FALSE;
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
    text_data *data = self->data;
    gchar *text = data->text->str;

    PangoLayout *layout;
    PangoFontDescription *desc = data->description;

    int w;
    int h;

    cairo_set_source_rgb(cr, data->font_color->r, data->font_color->g, data->font_color->b);

    layout = pango_cairo_create_layout (cr);
    pango_layout_set_text (layout, text, -1);
    pango_layout_set_font_description (layout, desc);
    pango_layout_get_size (layout, &w, &h);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    w = (double)w / PANGO_SCALE;
    h = (double)h / PANGO_SCALE;

    if (!self->width) self->width = w;
    if (!self->height) self->height = h;

    float factorw = (float)self->width / (float)w;
    float factorh = (float)self->height / (float)h;
    if (factorh == 1)
    {
        factorh = factorw;
        self->height = self->height * factorw;
    }

    cairo_matrix_t mx = {1, 0, 0, 1, 0, 0};
    tbo_object_get_flip_matrix (self, &mx);

    cairo_rectangle(cr, frame->x+2, frame->y+2, frame->width-4, frame->height-4);
    cairo_clip (cr);
    cairo_translate (cr, frame->x+self->x, frame->y+self->y);
    cairo_rotate (cr, self->angle);
    cairo_transform (cr, &mx);
    cairo_scale (cr, factorw, factorh);

    pango_cairo_show_layout (cr, layout);

    cairo_scale (cr, 1/factorw, 1/factorh);
    cairo_transform (cr, &mx);
    cairo_rotate (cr, -self->angle);
    cairo_translate (cr, -(frame->x+self->x), -(frame->y+self->y));
    cairo_reset_clip (cr);
}

char *
tbo_text_get_text (TextObj *self)
{
    return ((text_data *)self->data)->text->str;
}

void
tbo_text_set_text (TextObj *self, const char *text)
{
    g_string_assign (((text_data *)self->data)->text, text);
    self->height = 0;
}

void
tbo_text_get_color (TextObj *self, GdkColor *color)
{
    text_data *data = (text_data*)self->data;
    color->red = data->font_color->r * 65535;
    color->green = data->font_color->g * 65535;
    color->blue = data->font_color->b * 65535;
}

char *
tbo_text_get_string (TextObj *self)
{
    text_data *data = (text_data *)self->data;
    return pango_font_description_to_string (data->description);
}
