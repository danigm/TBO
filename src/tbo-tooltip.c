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


#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>
#include "tbo-drawing.h"
#include "tbo-tooltip.h"
#include "tbo-window.h"

#define TOOLTIP_ALPHA 0.7

static TboDrawing *
get_tooltip_drawing (TboWindow *tbo)
{
    if (tbo == NULL || tbo->drawing == NULL || !TBO_IS_DRAWING (tbo->drawing))
        return NULL;

    return TBO_DRAWING (tbo->drawing);
}

static void
clear_tooltip (TboDrawing *drawing, gboolean clear_timeout)
{
    if (drawing == NULL)
        return;

    if (clear_timeout && drawing->tooltip_timeout_id != 0)
    {
        g_source_remove (drawing->tooltip_timeout_id);
        drawing->tooltip_timeout_id = 0;
    }

    if (drawing->tooltip != NULL)
    {
        g_string_free (drawing->tooltip, TRUE);
        drawing->tooltip = NULL;
    }

    drawing->tooltip_x = 0;
    drawing->tooltip_y = 0;
    drawing->tooltip_alpha = 0.0;
}

void
cairo_rounded_rectangle (cairo_t *cr, int xx, int yy, int w, int h)
{
    double x             = xx,
           y             = yy,
           width         = w,
           height        = h,
           aspect        = 1.0,     /* aspect ratio */
           corner_radius = height / 10.0;   /* and corner curvature radius */

    double radius = corner_radius / aspect;
    double degrees = M_PI / 180.0;

    cairo_new_sub_path (cr);
    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);
}

gboolean
quit_tooltip_cb (gpointer p)
{
    TboDrawing *drawing = TBO_DRAWING (p);

    drawing->tooltip_timeout_id = 0;
    clear_tooltip (drawing, FALSE);
    tbo_drawing_update (drawing);
    return FALSE;
}

void
tbo_tooltip_draw_background (cairo_t *cr, int w, int h, TboDrawing *drawing)
{
    int margin = 5;

    cairo_set_source_rgba (cr, 0, 0, 0, drawing->tooltip_alpha);
    cairo_rounded_rectangle (cr, -margin, -margin, w + margin * 2, h + margin * 2);
    cairo_fill (cr);
}

void
tbo_tooltip_set (const char *tooltip, int x, int y, TboWindow *tbo)
{
    TboDrawing *drawing = get_tooltip_drawing (tbo);

    if (drawing == NULL)
        return;

    if (drawing->tooltip_timeout_id != 0)
    {
        g_source_remove (drawing->tooltip_timeout_id);
        drawing->tooltip_timeout_id = 0;
    }

    if (drawing->tooltip == NULL)
    {
        if (tooltip != NULL)
        {
            drawing->tooltip = g_string_new (tooltip);
            drawing->tooltip_alpha = TOOLTIP_ALPHA;
            drawing->tooltip_x = x;
            drawing->tooltip_y = y;
        }
    }
    else
    {
        if (tooltip != NULL &&
            x == drawing->tooltip_x &&
            y == drawing->tooltip_y &&
            !strcmp (tooltip, drawing->tooltip->str))
            return;

        if (tooltip == NULL)
        {
            clear_tooltip (drawing, FALSE);
        }
        else
        {
            g_string_free (drawing->tooltip, TRUE);
            drawing->tooltip = g_string_new (tooltip);
            drawing->tooltip_alpha = TOOLTIP_ALPHA;
            drawing->tooltip_x = x;
            drawing->tooltip_y = y;
        }
    }

    tbo_drawing_update (drawing);
}

void
tbo_tooltip_reset (TboWindow *tbo)
{
    clear_tooltip (get_tooltip_drawing (tbo), TRUE);
}

void
tbo_tooltip_draw (cairo_t *cr, TboDrawing *drawing)
{
    if (drawing == NULL || drawing->tooltip == NULL)
        return;

    int w=0, h=0;
    int posx, posy;
    gchar *text = drawing->tooltip->str;
    PangoLayout *layout;

    layout = pango_cairo_create_layout (cr);
    pango_layout_set_text (layout, text, -1);
    pango_layout_get_size (layout, &w, &h);
    w = (double)w / PANGO_SCALE;
    h = (double)h / PANGO_SCALE;

    //pango_layout_set_font_description (layout, desc);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);

    posx = drawing->tooltip_x - w / 2;
    posy = drawing->tooltip_y - h / 2;
    cairo_translate (cr, posx, posy);

    tbo_tooltip_draw_background (cr, w, h, drawing);

    cairo_set_source_rgba (cr, 1, 1, 1, drawing->tooltip_alpha);
    pango_cairo_show_layout (cr, layout);

    cairo_translate (cr, -posx, -posy);
    g_object_unref (layout);
}

void
tbo_tooltip_set_center_timeout (const char *tooltip, int timeout, TboWindow *tbo)
{
    TboDrawing *drawing = get_tooltip_drawing (tbo);
    int x, y;

    if (drawing == NULL)
        return;

    x = gtk_widget_get_width (tbo->drawing) / 2;
    y = gtk_widget_get_height (tbo->drawing) / 2;

    tbo_tooltip_set (tooltip, x, y, tbo);
    drawing->tooltip_timeout_id = g_timeout_add (timeout, quit_tooltip_cb, drawing);
}
