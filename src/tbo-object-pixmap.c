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
#include <stdint.h>
#include <cairo.h>
#include <gdk/gdk.h>
#include <stdio.h>
#include "tbo-types.h"
#include "frame.h"
#include "tbo-files.h"
#include "tbo-utils.h"
#include "tbo-object-pixmap.h"

G_DEFINE_TYPE (TboObjectPixmap, tbo_object_pixmap, TBO_TYPE_OBJECT_BASE);

static void draw (TboObjectBase *, Frame *, cairo_t *);
static void save (TboObjectBase *, FILE *);
static TboObjectBase * tclone (TboObjectBase *);

static gboolean
ensure_pixbuf (TboObjectPixmap *pixmap)
{
    GError *error = NULL;
    gchar *path;

    if (pixmap->pixbuf != NULL)
        return TRUE;

    path = tbo_files_expand_path (pixmap->path->str);
    pixmap->pixbuf = gdk_pixbuf_new_from_file (path, &error);
    if (pixmap->pixbuf == NULL)
    {
        if (error != NULL)
        {
            g_warning ("%s", error->message);
            g_error_free (error);
        }
        g_free (path);
        return FALSE;
    }

    g_free (path);
    return TRUE;
}

static gboolean
update_surface_cache (TboObjectPixmap *pixmap)
{
    cairo_surface_t *surface;
    unsigned char *data;
    int stride;
    int width;
    int height;
    int src_stride;
    int n_channels;
    guchar *src;
    int x;
    int y;
    gboolean has_alpha;

    if (pixmap->scaled_pixbuf == NULL)
        return FALSE;

    if (pixmap->surface != NULL)
    {
        cairo_surface_destroy (pixmap->surface);
        pixmap->surface = NULL;
    }

    width = gdk_pixbuf_get_width (pixmap->scaled_pixbuf);
    height = gdk_pixbuf_get_height (pixmap->scaled_pixbuf);
    src_stride = gdk_pixbuf_get_rowstride (pixmap->scaled_pixbuf);
    n_channels = gdk_pixbuf_get_n_channels (pixmap->scaled_pixbuf);
    has_alpha = gdk_pixbuf_get_has_alpha (pixmap->scaled_pixbuf);
    src = gdk_pixbuf_get_pixels (pixmap->scaled_pixbuf);

    if (n_channels < 3)
        return FALSE;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy (surface);
        return FALSE;
    }

    data = cairo_image_surface_get_data (surface);
    stride = cairo_image_surface_get_stride (surface);

    for (y = 0; y < height; y++)
    {
        uint32_t *dest = (uint32_t *) (data + (y * stride));
        guchar *row = src + (y * src_stride);

        for (x = 0; x < width; x++)
        {
            guchar *pixel = row + (x * n_channels);
            guchar r = pixel[0];
            guchar g = pixel[1];
            guchar b = pixel[2];
            guchar a = has_alpha && n_channels >= 4 ? pixel[3] : 255;

            if (a != 255)
            {
                r = (guchar) ((r * a + 127) / 255);
                g = (guchar) ((g * a + 127) / 255);
                b = (guchar) ((b * a + 127) / 255);
            }

            dest[x] = ((uint32_t) a << 24) |
                      ((uint32_t) r << 16) |
                      ((uint32_t) g << 8) |
                      (uint32_t) b;
        }
    }

    cairo_surface_mark_dirty (surface);
    pixmap->surface = surface;
    return TRUE;
}

static gboolean
ensure_scaled_pixbuf (TboObjectBase *self, TboObjectPixmap *pixmap)
{
    if (!ensure_pixbuf (pixmap))
        return FALSE;

    if (!self->width)
        self->width = gdk_pixbuf_get_width (pixmap->pixbuf);
    if (!self->height)
        self->height = gdk_pixbuf_get_height (pixmap->pixbuf);

    if (self->width <= 0 || self->height <= 0)
        return FALSE;

    if (pixmap->scaled_pixbuf != NULL &&
        pixmap->cache_width == self->width &&
        pixmap->cache_height == self->height)
        return TRUE;

    if (pixmap->scaled_pixbuf != NULL)
    {
        g_object_unref (pixmap->scaled_pixbuf);
        pixmap->scaled_pixbuf = NULL;
    }

    pixmap->scaled_pixbuf = gdk_pixbuf_scale_simple (pixmap->pixbuf,
                                                     self->width,
                                                     self->height,
                                                     GDK_INTERP_BILINEAR);
    if (pixmap->scaled_pixbuf == NULL)
        return FALSE;

    if (!update_surface_cache (pixmap))
    {
        g_object_unref (pixmap->scaled_pixbuf);
        pixmap->scaled_pixbuf = NULL;
        return FALSE;
    }

    pixmap->cache_width = self->width;
    pixmap->cache_height = self->height;
    return TRUE;
}

static void
draw (TboObjectBase *self, Frame *frame, cairo_t *cr)
{
    TboObjectPixmap *pixmap = TBO_OBJECT_PIXMAP (self);
    int frame_x = tbo_frame_get_x (frame);
    int frame_y = tbo_frame_get_y (frame);
    int frame_width = tbo_frame_get_width (frame);
    int frame_height = tbo_frame_get_height (frame);

    if (!ensure_scaled_pixbuf (self, pixmap))
        return;

    cairo_matrix_t mx = {1, 0, 0, 1, 0, 0};
    tbo_object_base_get_flip_matrix (self, &mx);

    cairo_rectangle (cr, frame_x + 2, frame_y + 2, frame_width - 4, frame_height - 4);
    cairo_clip (cr);
    cairo_translate (cr, frame_x + self->x, frame_y + self->y);
    cairo_rotate (cr, self->angle);
    cairo_transform (cr, &mx);

    cairo_set_source_surface (cr, pixmap->surface, 0, 0);
    cairo_paint (cr);

    cairo_transform (cr, &mx);
    cairo_rotate (cr, -self->angle);
    cairo_translate (cr, -(frame_x + self->x), -(frame_y + self->y));
    cairo_reset_clip (cr);
}

static void
save (TboObjectBase *self, FILE *file)
{
    GString *xml = g_string_new ("   <piximage");

    tbo_xml_append_object_attrs (xml, self);
    tbo_xml_append_attr_string (xml, "path", TBO_OBJECT_PIXMAP (self)->path->str);
    g_string_append (xml, ">\n ");
    tbo_xml_write (file, xml);
    fputs ("   </piximage>\n", file);
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
    self->pixbuf = NULL;
    self->scaled_pixbuf = NULL;
    self->surface = NULL;
    self->cache_width = 0;
    self->cache_height = 0;

    self->parent_instance.draw = draw;
    self->parent_instance.save = save;
    self->parent_instance.clone = tclone;
}

static void
tbo_object_pixmap_finalize (GObject *self)
{
    if (TBO_OBJECT_PIXMAP (self)->scaled_pixbuf)
        g_object_unref (TBO_OBJECT_PIXMAP (self)->scaled_pixbuf);
    if (TBO_OBJECT_PIXMAP (self)->surface)
        cairo_surface_destroy (TBO_OBJECT_PIXMAP (self)->surface);
    if (TBO_OBJECT_PIXMAP (self)->pixbuf)
        g_object_unref (TBO_OBJECT_PIXMAP (self)->pixbuf);
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
tbo_object_pixmap_new (void)
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
