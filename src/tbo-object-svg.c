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
#include <librsvg/rsvg-cairo.h>
#include "tbo-types.h"
#include "tbo-files.h"
#include "tbo-object-svg.h"

G_DEFINE_TYPE (TboObjectSvg, tbo_object_svg, TBO_TYPE_OBJECT_BASE);

static void draw (TboObjectBase *, Frame *, cairo_t *);
static void save (TboObjectBase *, FILE *);
static TboObjectBase * tclone (TboObjectBase *);

static void
draw (TboObjectBase *self, Frame *frame, cairo_t *cr)
{
    GError *error = NULL;
    RsvgHandle *rsvg_handle = NULL;
    RsvgDimensionData rsvg_dimension_data;
    TboObjectSvg *svg = TBO_OBJECT_SVG (self);
    char path[255];

    tbo_files_expand_path (svg->path->str, path);
    rsvg_handle = rsvg_handle_new_from_file (path, &error);
    if (!rsvg_handle)
    {
        g_print (_("Couldn't load %s\n"), path);
        return;
    }
    if (error != NULL)
    {
        g_print ("%s\n", error->message);
        g_error_free (error);
        return;
    }
    else
    {
        rsvg_handle_get_dimensions (rsvg_handle, &rsvg_dimension_data);
        int w = rsvg_dimension_data.width;
        int h = rsvg_dimension_data.height;
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

        rsvg_handle_render_cairo (rsvg_handle, cr);

        cairo_scale (cr, 1/factorw, 1/factorh);
        cairo_transform (cr, &mx);
        cairo_rotate (cr, -self->angle);
        cairo_translate (cr, -(frame->x+self->x), -(frame->y+self->y));
        cairo_reset_clip (cr);

        g_object_unref (rsvg_handle);
    }
}

static void
save (TboObjectBase *self, FILE *file)
{
    char buffer[1024];

    snprintf (buffer, 1024, "   <svgimage x=\"%d\" y=\"%d\" "
                           "width=\"%d\" height=\"%d\" "
                           "angle=\"%f\" flipv=\"%d\" fliph=\"%d\" "
                           "path=\"%s\">\n ",
                           self->x, self->y, self->width, self->height,
                           self->angle, self->flipv, self->fliph,
                           TBO_OBJECT_SVG (self)->path->str);
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    snprintf (buffer, 1024, "   </svgimage>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
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

    self->parent_instance.draw = draw;
    self->parent_instance.save = save;
    self->parent_instance.clone = tclone;
}

static void
tbo_object_svg_finalize (GObject *self)
{
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
tbo_object_svg_new ()
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
