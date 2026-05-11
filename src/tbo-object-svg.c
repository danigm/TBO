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
#include <string.h>
#include <math.h>
#include <cairo.h>
#include <stdio.h>
#include <librsvg/rsvg.h>
#include "tbo-types.h"
#include "frame.h"
#include "tbo-files.h"
#include "tbo-utils.h"
#include "tbo-object-svg.h"

G_DEFINE_TYPE (TboObjectSvg, tbo_object_svg, TBO_TYPE_OBJECT_BASE);

static void draw (TboObjectBase *, Frame *, cairo_t *);
static void save (TboObjectBase *, FILE *);
static TboObjectBase * tclone (TboObjectBase *);

static gboolean
ensure_handle (TboObjectSvg *svg)
{
    GError *error = NULL;
    gchar *path;

    if (svg->handle != NULL)
        return TRUE;

    path = tbo_files_expand_path (svg->path->str);
    svg->handle = rsvg_handle_new_from_file (path, &error);
    if (svg->handle == NULL)
    {
        if (error != NULL)
        {
            g_warning ("%s", error->message);
            g_error_free (error);
        }
        else
        {
            g_warning ("Couldn't load %s", path);
        }
        g_free (path);
        return FALSE;
    }

    g_free (path);
    return TRUE;
}

static gboolean
ensure_surface (TboObjectBase *self, TboObjectSvg *svg)
{
    GError *error = NULL;
    cairo_t *surface_cr;
    gdouble width_px = 0;
    gdouble height_px = 0;
    RsvgRectangle viewport;

    if (!ensure_handle (svg))
        return FALSE;

    if (!rsvg_handle_get_intrinsic_size_in_pixels (svg->handle, &width_px, &height_px))
    {
        width_px = self->width ? self->width : 128;
        height_px = self->height ? self->height : 128;
    }

    if (!self->width)
        self->width = ceil (width_px);
    if (!self->height)
        self->height = ceil (height_px);

    if (self->width <= 0 || self->height <= 0)
        return FALSE;

    if (svg->surface != NULL &&
        svg->cache_width == self->width &&
        svg->cache_height == self->height)
        return TRUE;

    if (svg->surface != NULL)
    {
        cairo_surface_destroy (svg->surface);
        svg->surface = NULL;
    }

    svg->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                               self->width,
                                               self->height);
    surface_cr = cairo_create (svg->surface);
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = self->width;
    viewport.height = self->height;
    rsvg_handle_render_document (svg->handle, surface_cr, &viewport, &error);
    cairo_destroy (surface_cr);

    if (error != NULL)
    {
        g_warning ("%s", error->message);
        g_error_free (error);
        cairo_surface_destroy (svg->surface);
        svg->surface = NULL;
        return FALSE;
    }

    svg->cache_width = self->width;
    svg->cache_height = self->height;
    return TRUE;
}

static void
draw (TboObjectBase *self, Frame *frame, cairo_t *cr)
{
    TboObjectSvg *svg = TBO_OBJECT_SVG (self);
    int frame_x = tbo_frame_get_x (frame);
    int frame_y = tbo_frame_get_y (frame);
    int frame_width = tbo_frame_get_width (frame);
    int frame_height = tbo_frame_get_height (frame);

    if (!ensure_surface (self, svg))
        return;

    cairo_matrix_t mx = {1, 0, 0, 1, 0, 0};
    tbo_object_base_get_flip_matrix (self, &mx);

    cairo_rectangle (cr, frame_x + 2, frame_y + 2, frame_width - 4, frame_height - 4);
    cairo_clip (cr);
    cairo_translate (cr, frame_x + self->x, frame_y + self->y);
    cairo_rotate (cr, self->angle);
    cairo_transform (cr, &mx);
    cairo_set_source_surface (cr, svg->surface, 0, 0);
    cairo_paint (cr);

    cairo_transform (cr, &mx);
    cairo_rotate (cr, -self->angle);
    cairo_translate (cr, -(frame_x + self->x), -(frame_y + self->y));
    cairo_reset_clip (cr);
}

static void
save (TboObjectBase *self, FILE *file)
{
    GString *xml = g_string_new ("   <svgimage");

    tbo_xml_append_object_attrs (xml, self);
    tbo_xml_append_attr_string (xml, "path", TBO_OBJECT_SVG (self)->path->str);
    g_string_append (xml, ">\n ");
    tbo_xml_write (file, xml);
    fputs ("   </svgimage>\n", file);
}

static TboObjectBase *
tclone (TboObjectBase *self)
{
    TboObjectSvg *svg;
    TboObjectBase *newsvg;
    svg = TBO_OBJECT_SVG (self);

    newsvg = TBO_OBJECT_BASE (tbo_object_svg_new_with_params (self->x,
                                                              self->y,
                                                              self->width,
                                                              self->height,
                                                              svg->path->str));
    newsvg->angle = self->angle;
    newsvg->flipv = self->flipv;
    newsvg->fliph = self->fliph;

    return newsvg;
}

/* init methods */

static void
tbo_object_svg_init (TboObjectSvg *self)
{
    self->path = NULL;
    self->handle = NULL;
    self->surface = NULL;
    self->cache_width = 0;
    self->cache_height = 0;

    self->parent_instance.draw = draw;
    self->parent_instance.save = save;
    self->parent_instance.clone = tclone;
}

static void
tbo_object_svg_finalize (GObject *self)
{
    if (TBO_OBJECT_SVG (self)->surface)
        cairo_surface_destroy (TBO_OBJECT_SVG (self)->surface);
    if (TBO_OBJECT_SVG (self)->handle)
        g_object_unref (TBO_OBJECT_SVG (self)->handle);
    if (TBO_OBJECT_SVG (self)->path)
        g_string_free (TBO_OBJECT_SVG (self)->path, TRUE);
    /* Chain up to the parent class */
    G_OBJECT_CLASS (tbo_object_svg_parent_class)->finalize (self);
}

static void
tbo_object_svg_class_init (TboObjectSvgClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = tbo_object_svg_finalize;
}

/* object functions */

GObject *
tbo_object_svg_new (void)
{
    GObject *tbo_object;
    TboObjectSvg *svg;
    tbo_object = g_object_new (TBO_TYPE_OBJECT_SVG, NULL);
    svg = TBO_OBJECT_SVG (tbo_object);
    svg->path = g_string_new ("");

    return tbo_object;
}

GObject *
tbo_object_svg_new_with_params (gint   x,
                                gint   y,
                                gint   width,
                                gint   height,
                                gchar *path)
{
    TboObjectSvg *svg;
    TboObjectBase *obj;
    svg = TBO_OBJECT_SVG (tbo_object_svg_new ());
    obj = TBO_OBJECT_BASE (svg);
    obj->x = x;
    obj->y = y;
    obj->width = width;
    obj->height = height;
    g_string_assign (svg->path, path);

    return G_OBJECT (svg);
}
