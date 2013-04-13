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
#include <string.h>
#include <cairo.h>
#include <gdk/gdk.h>
#include <stdio.h>
#include "tbo-types.h"
#include "tbo-object-pixmap.h"

G_DEFINE_TYPE (TboObjectPixmap, tbo_object_pixmap, TBO_TYPE_OBJECT_BASE);

static void draw (TboObjectBase *, Frame *, cairo_t *);
static void save (TboObjectBase *, FILE *);
static TboObjectBase * tclone (TboObjectBase *);

static void
draw (TboObjectBase *self, Frame *frame, cairo_t *cr)
{
    TboObjectPixmap *pixmap = TBO_OBJECT_PIXMAP (self);
    int w, h;
    cairo_surface_t *image;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    char path[255];

    tbo_files_expand_path (pixmap->path->str, path);
    pixbuf = gdk_pixbuf_new_from_file (path, &error);

    if (!pixbuf) {
        g_warning ("There's a problem here: %s", error->message);
        return;
    }

    w = gdk_pixbuf_get_width (pixbuf);
    h = gdk_pixbuf_get_height (pixbuf);

    if (!self->width) self->width = w;
    if (!self->height) self->height = h;

    float factorw = (float)self->width / (float)w;
    float factorh = (float)self->height / (float)h;

    cairo_matrix_t mx = {1, 0, 0, 1, 0, 0};
    tbo_object_base_get_flip_matrix (self, &mx);

    cairo_rectangle(cr, frame->x+2, frame->y+2, frame->width-4, frame->height-4);
    cairo_clip (cr);
    cairo_translate (cr, frame->x+self->x, frame->y+self->y);
    cairo_rotate (cr, self->angle);
    cairo_transform (cr, &mx);
    cairo_scale (cr, factorw, factorh);

    gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
    cairo_paint (cr);

    cairo_scale (cr, 1/factorw, 1/factorh);
    cairo_transform (cr, &mx);
    cairo_rotate (cr, -self->angle);
    cairo_translate (cr, -(frame->x+self->x), -(frame->y+self->y));
    cairo_reset_clip (cr);

    cairo_surface_destroy (image);
}

static void
save (TboObjectBase *self, FILE *file)
{
    char buffer[1024];

    snprintf (buffer, 1024, "   <piximage x=\"%d\" y=\"%d\" "
                           "width=\"%d\" height=\"%d\" "
                           "angle=\"%f\" flipv=\"%d\" fliph=\"%d\" "
                           "path=\"%s\">\n ",
                           self->x, self->y, self->width, self->height,
                           self->angle, self->flipv, self->fliph, TBO_OBJECT_PIXMAP (self)->path->str);
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    snprintf (buffer, 1024, "   </piximage>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
}

static TboObjectBase *
tclone (TboObjectBase *self)
{
    TboObjectPixmap *pixmap;
    TboObjectBase *newpixmap;
    pixmap = TBO_OBJECT_PIXMAP (self);

    newpixmap = TBO_OBJECT_BASE (tbo_object_pixmap_new_with_params (self->x,
                                                                    self->y,
                                                                    self->width,
                                                                    self->height,
                                                                    pixmap->path->str));
    newpixmap->angle = self->angle;
    newpixmap->flipv = self->flipv;
    newpixmap->fliph = self->fliph;

    return newpixmap;
}

/* init methods */

static void
tbo_object_pixmap_init (TboObjectPixmap *self)
{
    self->path = NULL;

    self->parent_instance.draw = draw;
    self->parent_instance.save = save;
    self->parent_instance.clone = tclone;
}

static void
tbo_object_pixmap_finalize (GObject *self)
{
    if (TBO_OBJECT_PIXMAP (self)->path)
        g_string_free (TBO_OBJECT_PIXMAP (self)->path, TRUE);
    /* Chain up to the parent class */
    G_OBJECT_CLASS (tbo_object_pixmap_parent_class)->finalize (self);
}

static void
tbo_object_pixmap_class_init (TboObjectPixmapClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = tbo_object_pixmap_finalize;
}

/* object functions */

GObject *
tbo_object_pixmap_new ()
{
    GObject *tbo_object;
    TboObjectPixmap *pixmap;
    tbo_object = g_object_new (TBO_TYPE_OBJECT_PIXMAP, NULL);
    pixmap = TBO_OBJECT_PIXMAP (tbo_object);
    pixmap->path = g_string_new ("");

    return tbo_object;
}

GObject *
tbo_object_pixmap_new_with_params (gint   x,
                                   gint   y,
                                   gint   width,
                                   gint   height,
                                   gchar *path)
{
    TboObjectPixmap *pixmap;
    TboObjectBase *obj;
    pixmap = TBO_OBJECT_PIXMAP (tbo_object_pixmap_new ());
    obj = TBO_OBJECT_BASE (pixmap);
    obj->x = x;
    obj->y = y;
    obj->width = width;
    obj->height = height;
    g_string_assign (pixmap->path, path);

    return G_OBJECT (pixmap);
}

