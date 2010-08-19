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


#include <math.h>
#include "frame.h"
#include "tbo-tool-frame.h"
#include "tbo-drawing.h"

G_DEFINE_TYPE (TboToolFrame, tbo_tool_frame, TBO_TYPE_TOOL_BASE);

#define MINIMUM(x, y) x < y ? (int)x : (int)y

/* Headers */
static void on_move (TboToolBase *tool, GtkWidget *widget, GdkEventMotion *event);
static void on_click (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event);
static void on_release (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event);
static void drawing (TboToolBase *tool, cairo_t *cr);

/* Definitions */

/* tool signal */
static void
on_move (TboToolBase *tool, GtkWidget *widget, GdkEventMotion *event)
{
    int x, y;
    TboToolFrame *self = TBO_TOOL_FRAME (tool);

    if (self->n_frame_x >= 0)
    {
        if (!self->tmp_frame)
        {
            self->tmp_frame = tbo_frame_new (
                    self->n_frame_x,
                    self->n_frame_y,
                    (int)fabs ((int)event->x - self->n_frame_x),
                    (int)fabs ((int)event->y - self->n_frame_y));
        }
        else
        {
            x = (int)event->x;
            y = (int)event->y;
            self->tmp_frame->width = (int)fabs (x - self->n_frame_x);
            self->tmp_frame->height = (int)fabs (y - self->n_frame_y);
            self->tmp_frame->x = MINIMUM (self->n_frame_x, x);
            self->tmp_frame->y = MINIMUM (self->n_frame_y, y);
        }
    }
    tbo_drawing_update (TBO_DRAWING (tool->tbo->drawing));
}

static void
on_click (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event)
{
    TboToolFrame *self = TBO_TOOL_FRAME (tool);
    self->n_frame_x = (int)event->x;
    self->n_frame_y = (int)event->y;
}

static void
on_release (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event)
{
    int w, h;
    TboWindow *tbo = tool->tbo;
    TboToolFrame *self = TBO_TOOL_FRAME (tool);

    w = (int)fabs (event->x - self->n_frame_x);
    h = (int)fabs (event->y - self->n_frame_y);

    if (w != 0 && h != 0)
    {
        tbo_page_new_frame (tbo_comic_get_current_page (tbo->comic),
                MINIMUM (self->n_frame_x, event->x), MINIMUM (self->n_frame_y, event->y),
                w, h);
    }

    self->n_frame_x = -1;
    self->n_frame_y = -1;
    if (self->tmp_frame) tbo_frame_free (self->tmp_frame);
    self->tmp_frame = NULL;
    tbo_drawing_update (TBO_DRAWING (tool->tbo->drawing));
}

static void
drawing (TboToolBase *tool, cairo_t *cr)
{
    TboToolFrame *self = TBO_TOOL_FRAME (tool);
    if (self->tmp_frame != NULL)
    {
        tbo_frame_draw_complete (self->tmp_frame, cr, 
                1, 1, 1,
                1, 0, 0,
                2);
    }
}

/* init methods */

static void
tbo_tool_frame_init (TboToolFrame *self)
{
    self->n_frame_x = -1;
    self->n_frame_y = -1;
    self->tmp_frame = NULL;

    self->parent_instance.on_move = on_move;
    self->parent_instance.on_click = on_click;
    self->parent_instance.on_release = on_release;
    self->parent_instance.drawing = drawing;
}

static void
tbo_tool_frame_class_init (TboToolFrameClass *klass)
{
}

/* object functions */

GObject *
tbo_tool_frame_new ()
{
    GObject *tbo_tool;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_FRAME, NULL);
    return tbo_tool;
}

GObject *
tbo_tool_frame_new_with_params (TboWindow *tbo)
{
    GObject *tbo_tool;
    TboToolBase *tbo_tool_base;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_FRAME, NULL);
    tbo_tool_base = TBO_TOOL_BASE (tbo_tool);
    tbo_tool_base->tbo = tbo;
    return tbo_tool;
}

