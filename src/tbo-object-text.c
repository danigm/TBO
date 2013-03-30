/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include "tbo-types.h"
#include "tbo-object-text.h"

G_DEFINE_TYPE (TboObjectText, tbo_object_text, TBO_TYPE_OBJECT_BASE);

static void draw (TboObjectBase *, Frame *, cairo_t *);
static void save (TboObjectBase *, FILE *);
static TboObjectBase * clone (TboObjectBase *);

static void
draw (TboObjectBase *self, Frame *frame, cairo_t *cr)
{
    TboObjectText *textobj = TBO_OBJECT_TEXT (self);
    gchar *text = textobj->text->str;

    PangoLayout *layout;
    PangoFontDescription *desc = textobj->description;

    int w;
    int h;

    if (!strlen(text)) {
        return;
    }

    gdk_cairo_set_source_color (cr, textobj->font_color);

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
    tbo_object_base_get_flip_matrix (self, &mx);

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

static void
save (TboObjectBase *self, FILE *file)
{
    char buffer[1024];
    float r, g, b;

    TboObjectText *text = TBO_OBJECT_TEXT (self);

    r = (float)text->font_color->red / (float)COLORMAX;
    g = (float)text->font_color->green / (float)COLORMAX;
    b = (float)text->font_color->blue / (float)COLORMAX;

    snprintf (buffer, 1024, "   <text x=\"%d\" y=\"%d\" "
                           "width=\"%d\" height=\"%d\" "
                           "angle=\"%f\" flipv=\"%d\" fliph=\"%d\" "
                           "font=\"%s\" "
                           "r=\"%f\" g=\"%f\" b=\"%f\">\n",
                           self->x, self->y, self->width, self->height,
                           self->angle, self->flipv, self->fliph,
                           pango_font_description_to_string (text->description),
                           r, g, b);
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    snprintf (buffer, 1024, "%s", g_markup_escape_text (text->text->str, strlen (text->text->str)));
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    snprintf (buffer, 1024, "\n   </text>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
}

static TboObjectBase *
clone (TboObjectBase *self)
{
    TboObjectText *text;
    TboObjectBase *newtext;
    text = TBO_OBJECT_TEXT (self);

    newtext = TBO_OBJECT_BASE (tbo_object_text_new_with_params (self->x,
                                                                self->y,
                                                                self->width,
                                                                self->height,
                                                                text->text->str,
                                                                tbo_object_text_get_string (text),
                                                                text->font_color));
    newtext->angle = self->angle;
    newtext->flipv = self->flipv;
    newtext->fliph = self->fliph;

    return newtext;
}

/* init methods */

static void
tbo_object_text_init (TboObjectText *self)
{
    self->text = NULL;
    self->description = NULL;
    self->font_color = NULL;

    self->parent_instance.draw = draw;
    self->parent_instance.save = save;
    self->parent_instance.clone = clone;
}

static void
tbo_object_text_finalize (GObject *self)
{
    TboObjectText *text = TBO_OBJECT_TEXT (self);
    if (text->text)
        g_string_free (text->text, TRUE);
    if (text->description)
        pango_font_description_free (text->description);
    if (text->font_color)
        gdk_color_free (text->font_color);
    /* Chain up to the parent class */
    G_OBJECT_CLASS (tbo_object_text_parent_class)->finalize (self);
}

static void
tbo_object_text_class_init (TboObjectTextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = tbo_object_text_finalize;
}

/* object functions */

GObject *
tbo_object_text_new ()
{
    GObject *tbo_object;
    TboObjectText *text;
    GdkColor color = { 0, 0, 0, 0 };
    tbo_object = g_object_new (TBO_TYPE_OBJECT_TEXT, NULL);
    text = TBO_OBJECT_TEXT (tbo_object);
    text->text = g_string_new (_("text"));
    text->description = pango_font_description_from_string ("Sans Normal 27");
    text->font_color = gdk_color_copy (&color);

    return tbo_object;
}

GObject *
tbo_object_text_new_with_params (gint     x,
                                 gint     y,
                                 gint     width,
                                 gint     height,
                                 gchar    *text,
                                 gchar    *fontname,
                                 GdkColor *color)
{
    TboObjectBase *obj;
    TboObjectText *textobj;

    obj = TBO_OBJECT_BASE (tbo_object_text_new ());
    textobj = TBO_OBJECT_TEXT (obj);
    obj->x = x;
    obj->y = y;
    obj->width = width;
    obj->height = height;

    g_string_assign (textobj->text, text);
    textobj->description = pango_font_description_from_string (fontname);
    textobj->font_color = gdk_color_copy (color);

    return G_OBJECT (obj);
}

gchar *
tbo_object_text_get_text (TboObjectText *self)
{
    return self->text->str;
}

void
tbo_object_text_set_text (TboObjectText *self, const gchar *text)
{
    g_string_assign (self->text, text);
    /* auto fit width & height */
    TBO_OBJECT_BASE (self)->height = 0;
}

void
tbo_object_text_change_font (TboObjectText *self, gchar *font)
{
    if (self->description)
        pango_font_description_free (self->description);
    self->description = pango_font_description_from_string (font);
}

void
tbo_object_text_change_color (TboObjectText *self, GdkColor *color)
{
    if (self->font_color)
        gdk_color_free (self->font_color);
    self->font_color = gdk_color_copy (color);
}

gchar *
tbo_object_text_get_string (TboObjectText *self)
{
    return pango_font_description_to_string (self->description);
}
