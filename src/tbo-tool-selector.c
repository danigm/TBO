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
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-window.h"
#include "tbo-ui-utils.h"
#include "tbo-widget.h"
#include "tbo-tool-selector.h"
#include "tbo-drawing.h"
#include "tbo-object-group.h"
#include "tbo-tool-text.h"
#include "ui-menu.h"
#include "tbo-tooltip.h"
#include "tbo-undo.h"

G_DEFINE_TYPE (TboToolSelector, tbo_tool_selector, TBO_TYPE_TOOL_BASE);

/* Headers */
static void on_select (TboToolBase *tool);
static void on_unselect (TboToolBase *tool);
static void on_move (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event);
static void on_click (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event);
static void on_release (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event);
static void on_key (TboToolBase *tool, GtkWidget *widget, TboKeyEvent event);
static void drawing (TboToolBase *tool, cairo_t *cr);

static void frame_view_on_move (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event);
static void page_view_on_move (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event);
static void frame_view_on_click (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event);
static void page_view_on_click (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event);
static void frame_view_drawing (TboToolBase *tool, cairo_t *cr);
static void page_view_drawing (TboToolBase *tool, cairo_t *cr);
static void frame_view_on_key (TboToolBase *tool, GtkWidget *widget, TboKeyEvent event);
static gboolean delete_selected (TboToolSelector *self);
static void open_text_editor (TboToolSelector *self, TboObjectText *text);
static void finalize (GObject *object);
static void tbo_tool_selector_set_selected_frame_pointer (TboToolSelector *self, Frame *frame);
static void tbo_tool_selector_set_selected_object_pointer (TboToolSelector *self, TboObjectBase *obj);
static void clear_selected_group (TboToolSelector *self);

#define MIN_FRAME_DIMENSION 1
#define MIN_OBJECT_DIMENSION 1
#define ANGLE_EPSILON 1e-9

static gint
clamp_frame_dimension (gint value)
{
    return MAX (MIN_FRAME_DIMENSION, value);
}

static gint
clamp_object_dimension (gint value)
{
    return MAX (MIN_OBJECT_DIMENSION, value);
}

static gboolean
frame_geometry_changed (TboToolSelector *tool)
{
    Frame *frame = tool->selected_frame;

    return frame != NULL &&
           (tool->start_m_x != tbo_frame_get_x (frame) ||
            tool->start_m_y != tbo_frame_get_y (frame) ||
            tool->start_m_w != tbo_frame_get_width (frame) ||
            tool->start_m_h != tbo_frame_get_height (frame));
}

static gboolean
object_geometry_changed (TboToolSelector *tool)
{
    TboObjectBase *obj = tool->selected_object;

    return obj != NULL &&
           (tool->start_m_x != obj->x ||
            tool->start_m_y != obj->y ||
            tool->start_m_w != obj->width ||
            tool->start_m_h != obj->height ||
            fabs (tool->start_m_angle - obj->angle) > ANGLE_EPSILON);
}

/* Definitions */

/* aux */
static gboolean
update_selected_cb (GtkSpinButton *widget, TboToolSelector *tool)
{
    TboDrawing *drawing = TBO_DRAWING (TBO_TOOL_BASE (tool)->tbo->drawing);
    TboWindow *tbo = TBO_TOOL_BASE (tool)->tbo;
    GdkRGBA old_color;
    gint old_x;
    gint old_y;
    gint old_width;
    gint old_height;
    gboolean old_border;
    gint x;
    gint y;
    gint width;
    gint height;

    if (tool->resizing || tool->clicked || tool->selected_frame == NULL || tool->spin_x == NULL)
        return FALSE;

    x = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (tool->spin_x));
    y = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (tool->spin_y));
    width = clamp_frame_dimension (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (tool->spin_w)));
    height = clamp_frame_dimension (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (tool->spin_h)));

    if (x == tbo_frame_get_x (tool->selected_frame) &&
        y == tbo_frame_get_y (tool->selected_frame) &&
        width == tbo_frame_get_width (tool->selected_frame) &&
        height == tbo_frame_get_height (tool->selected_frame))
        return FALSE;

    old_x = tbo_frame_get_x (tool->selected_frame);
    old_y = tbo_frame_get_y (tool->selected_frame);
    old_width = tbo_frame_get_width (tool->selected_frame);
    old_height = tbo_frame_get_height (tool->selected_frame);
    old_border = tbo_frame_get_border (tool->selected_frame);
    tbo_frame_get_color (tool->selected_frame, &old_color);

    tbo_frame_set_bounds (tool->selected_frame,
                          x,
                          y,
                          width,
                          height);

    tbo_undo_stack_insert (tbo->undo_stack,
                           tbo_action_frame_state_new (tool->selected_frame,
                                                       old_x,
                                                       old_y,
                                                       old_width,
                                                       old_height,
                                                       old_border,
                                                       old_color.red,
                                                       old_color.green,
                                                       old_color.blue,
                                                       x,
                                                       y,
                                                       width,
                                                       height,
                                                       old_border,
                                                       old_color.red,
                                                       old_color.green,
                                                       old_color.blue));
    tbo_window_mark_dirty (tbo);
    tbo_toolbar_update (tbo->toolbar);
    tbo_drawing_update (drawing);
    return FALSE;
}

static void
tbo_tool_selector_set_selected_frame_pointer (TboToolSelector *self, Frame *frame)
{
    if (self->selected_frame == frame)
        return;

    if (self->selected_frame != NULL)
    {
        g_object_remove_weak_pointer (G_OBJECT (self->selected_frame),
                                      (gpointer *) &self->selected_frame);
    }

    self->selected_frame = frame;

    if (self->selected_frame != NULL)
    {
        g_object_add_weak_pointer (G_OBJECT (self->selected_frame),
                                   (gpointer *) &self->selected_frame);
    }
}

static void
tbo_tool_selector_set_selected_object_pointer (TboToolSelector *self, TboObjectBase *obj)
{
    if (self->selected_object == obj)
        return;

    if (self->selected_object != NULL)
    {
        g_object_remove_weak_pointer (G_OBJECT (self->selected_object),
                                      (gpointer *) &self->selected_object);
    }

    self->selected_object = obj;

    if (self->selected_object != NULL)
    {
        g_object_add_weak_pointer (G_OBJECT (self->selected_object),
                                   (gpointer *) &self->selected_object);
    }
}

static void
clear_selected_group (TboToolSelector *self)
{
    if (!TBO_IS_OBJECT_GROUP (self->selected_object) || self->selected_frame == NULL)
        return;

    if (tbo_frame_has_obj (self->selected_frame, self->selected_object))
        tbo_frame_del_obj (self->selected_frame, self->selected_object);
}

static void
update_color_cb (GtkWidget *button, GParamSpec *pspec, TboToolSelector *tool)
{
    TboDrawing *drawing = TBO_DRAWING (TBO_TOOL_BASE (tool)->tbo->drawing);
    TboWindow *tbo = TBO_TOOL_BASE (tool)->tbo;
    GdkRGBA current_color;
    gboolean border;
    if (tool->resizing || tool->clicked || tool->selected_frame == NULL)
        return;

    GdkRGBA color = tbo_color_picker_get_rgba (button);
    tbo_frame_get_color (tool->selected_frame, &current_color);
    if (gdk_rgba_equal (&current_color, &color))
        return;

    border = tbo_frame_get_border (tool->selected_frame);

    tbo_frame_set_color (tool->selected_frame, &color);
    tbo_undo_stack_insert (tbo->undo_stack,
                           tbo_action_frame_state_new (tool->selected_frame,
                                                       tbo_frame_get_x (tool->selected_frame),
                                                       tbo_frame_get_y (tool->selected_frame),
                                                       tbo_frame_get_width (tool->selected_frame),
                                                       tbo_frame_get_height (tool->selected_frame),
                                                       border,
                                                       current_color.red,
                                                       current_color.green,
                                                       current_color.blue,
                                                       tbo_frame_get_x (tool->selected_frame),
                                                       tbo_frame_get_y (tool->selected_frame),
                                                       tbo_frame_get_width (tool->selected_frame),
                                                       tbo_frame_get_height (tool->selected_frame),
                                                       border,
                                                       color.red,
                                                       color.green,
                                                       color.blue));
    tbo_window_mark_dirty (tbo);
    tbo_toolbar_update (tbo->toolbar);
    tbo_drawing_update (drawing);
}

static gboolean
update_border_cb (GtkCheckButton *button, TboToolSelector *tool)
{
    TboDrawing *drawing = TBO_DRAWING (TBO_TOOL_BASE (tool)->tbo->drawing);
    TboWindow *tbo = TBO_TOOL_BASE (tool)->tbo;
    gboolean border;
    GdkRGBA color;
    if (tool->resizing || tool->clicked || tool->selected_frame == NULL)
        return FALSE;

    border = gtk_check_button_get_active (button);
    if (tbo_frame_get_border (tool->selected_frame) == border)
        return FALSE;

    tbo_frame_get_color (tool->selected_frame, &color);

    tbo_frame_set_border (tool->selected_frame, border);
    tbo_undo_stack_insert (tbo->undo_stack,
                           tbo_action_frame_state_new (tool->selected_frame,
                                                       tbo_frame_get_x (tool->selected_frame),
                                                       tbo_frame_get_y (tool->selected_frame),
                                                       tbo_frame_get_width (tool->selected_frame),
                                                       tbo_frame_get_height (tool->selected_frame),
                                                       !border,
                                                       color.red,
                                                       color.green,
                                                       color.blue,
                                                       tbo_frame_get_x (tool->selected_frame),
                                                       tbo_frame_get_y (tool->selected_frame),
                                                       tbo_frame_get_width (tool->selected_frame),
                                                       tbo_frame_get_height (tool->selected_frame),
                                                       border,
                                                       color.red,
                                                       color.green,
                                                       color.blue));
    tbo_window_mark_dirty (tbo);
    tbo_toolbar_update (tbo->toolbar);
    tbo_drawing_update (drawing);
    return FALSE;
}

static void
update_tool_area (TboToolSelector *self)
{
    GtkWidget *toolarea = self->toolarea_widget;
    GtkWidget *hpanel;
    GtkWidget *label;
    GdkRGBA gdk_color = { 0, 0, 0, 1 };
    int frame_x, frame_y, frame_width, frame_height;

    tbo_frame_get_bounds (self->selected_frame, &frame_x, &frame_y, &frame_width, &frame_height);
    tbo_frame_get_color (self->selected_frame, &gdk_color);

    if (!self->spin_x)
    {
        self->spin_x = add_spin_with_label (toolarea, "x: ", frame_x);
        self->spin_y = add_spin_with_label (toolarea, "y: ", frame_y);
        self->spin_w = add_spin_with_label (toolarea, "w: ", frame_width);
        self->spin_h = add_spin_with_label (toolarea, "h: ", frame_height);
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (self->spin_x), -10000, 10000);
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (self->spin_y), -10000, 10000);
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (self->spin_w), MIN_FRAME_DIMENSION, 10000);
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (self->spin_h), MIN_FRAME_DIMENSION, 10000);

        g_signal_connect (self->spin_x, "value-changed", G_CALLBACK (update_selected_cb), self);
        g_signal_connect (self->spin_y, "value-changed", G_CALLBACK (update_selected_cb), self);
        g_signal_connect (self->spin_w, "value-changed", G_CALLBACK (update_selected_cb), self);
        g_signal_connect (self->spin_h, "value-changed", G_CALLBACK (update_selected_cb), self);

        hpanel = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
        label = gtk_label_new (_("Background color: "));
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_label_set_yalign (GTK_LABEL (label), 0.5);
        self->color_button = tbo_color_picker_new (&gdk_color);

        tbo_box_pack_start (hpanel, label, TRUE, TRUE, 5);
        tbo_box_pack_start (hpanel, self->color_button, TRUE, TRUE, 5);
        tbo_box_pack_start (toolarea, hpanel, FALSE, FALSE, 5);
        g_signal_connect (self->color_button, "notify::rgba", G_CALLBACK (update_color_cb), self);

        self->border_button = gtk_check_button_new_with_label (_("border"));
        gtk_check_button_set_active (GTK_CHECK_BUTTON (self->border_button), tbo_frame_get_border (self->selected_frame));
        tbo_box_pack_start (toolarea, self->border_button, FALSE, FALSE, 5);
        g_signal_connect (self->border_button, "toggled", G_CALLBACK (update_border_cb), self);

        tbo_widget_show_all (toolarea);
    }

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->spin_x), frame_x);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->spin_y), frame_y);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->spin_w), frame_width);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->spin_h), frame_height);
    gtk_check_button_set_active (GTK_CHECK_BUTTON (self->border_button), tbo_frame_get_border (self->selected_frame));
    tbo_color_picker_set_rgba (self->color_button, &gdk_color);
}

static void
on_select (TboToolBase *tool)
{
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);

    if (self->toolarea_widget == NULL)
    {
        self->toolarea_widget = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        g_object_ref_sink (self->toolarea_widget);
    }

    tbo_empty_tool_area (tool->tbo);
    tbo_widget_add_child (tool->tbo->toolarea, self->toolarea_widget);

    if (self->selected_frame != NULL)
        update_tool_area (self);
    else
        update_menubar (tool->tbo);
}

static void
on_unselect (TboToolBase *tool)
{
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);

    if (self->toolarea_widget != NULL &&
        gtk_widget_get_parent (self->toolarea_widget) == GTK_WIDGET (tool->tbo->toolarea))
    {
        tbo_widget_remove_child (tool->tbo->toolarea, self->toolarea_widget);
    }
}

static gboolean
over_resizer (TboToolSelector *self, Frame *frame, int x, int y)
{
    int rx, ry;
    rx = tbo_frame_get_x (frame) + tbo_frame_get_width (frame);
    ry = tbo_frame_get_y (frame) + tbo_frame_get_height (frame);

    float r_size;
    r_size = R_SIZE / tbo_drawing_get_zoom (TBO_DRAWING (TBO_TOOL_BASE (self)->tbo->drawing));

    if (((rx-r_size) < x) &&
        ((rx+r_size) > x) &&
        ((ry-r_size) < y) &&
        ((ry+r_size) > y))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static gboolean
over_resizer_obj (TboToolSelector *self, TboObjectBase *obj, int x, int y)
{
    int rx, ry;
    int ox, oy, ow, oh;
    TboDrawing *drawing = TBO_DRAWING (TBO_TOOL_BASE (self)->tbo->drawing);

    tbo_drawing_get_object_relative (drawing, obj, &ox, &oy, &ow, &oh);
    rx = ox + (ow * cos(obj->angle) - oh * sin(obj->angle));
    ry = oy + (oh * cos(obj->angle) + ow * sin(obj->angle));

    float r_size;
    r_size = R_SIZE / tbo_drawing_get_zoom (TBO_DRAWING (TBO_TOOL_BASE (self)->tbo->drawing));

    if (((rx-r_size) < x) &&
        ((rx+r_size) > x) &&
        ((ry-r_size) < y) &&
        ((ry+r_size) > y))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static gboolean
over_rotater_obj (TboToolSelector *self, TboObjectBase *obj, int x, int y)
{
    int rx, ry;
    int ox, oy, ow, oh;
    TboDrawing *drawing = TBO_DRAWING (TBO_TOOL_BASE (self)->tbo->drawing);

    tbo_drawing_get_object_relative (drawing, obj, &ox, &oy, &ow, &oh);
    rx = ox;
    ry = oy;

    float r_size;
    r_size = R_SIZE / tbo_drawing_get_zoom (TBO_DRAWING (TBO_TOOL_BASE (self)->tbo->drawing));

    if (((rx-r_size/2.0) < x) &&
        ((rx+r_size/2.0) > x) &&
        ((ry-r_size/2.0) < y) &&
        ((ry+r_size/2.0) > y))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static gboolean
moved_object (TboToolSelector *tool)
{
    TboObjectBase *obj = tool->selected_object;
    return (tool->start_m_x != obj->x || tool->start_m_y != obj->y);
}

/* tool signal */
static void
on_move (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event)
{
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);
    Frame *frame = tbo_drawing_get_current_frame (drawing);
    if (frame)
        frame_view_on_move (tool, widget, event);
    else
        page_view_on_move (tool, widget, event);

    tbo_drawing_update (drawing);
}

static void
on_click (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event)
{
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);
    Frame *frame = tbo_drawing_get_current_frame (drawing);
    if (frame)
        frame_view_on_click (tool, widget, event);
    else
        page_view_on_click (tool, widget, event);

    tbo_drawing_update (drawing);
}

static void
on_release (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event)
{
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    TboWindow *tbo = tool->tbo;
    gboolean should_open_text_editor = FALSE;
    // TODO create undo actions for movements / resizing and rotating
    if (object_geometry_changed (self)) {
        tbo_undo_stack_insert (tbo->undo_stack,
                               tbo_action_object_transform_new (self->selected_object,
                                                                self->start_m_x,
                                                                self->start_m_y,
                                                                self->start_m_w,
                                                                self->start_m_h,
                                                                self->start_m_angle,
                                                                self->selected_object->x,
                                                                self->selected_object->y,
                                                                self->selected_object->width,
                                                                 self->selected_object->height,
                                                                 self->selected_object->angle));
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
    }
    else if (frame_geometry_changed (self)) {
        tbo_undo_stack_insert (tbo->undo_stack,
                               tbo_action_frame_transform_new (self->selected_frame,
                                                               self->start_m_x,
                                                               self->start_m_y,
                                                               self->start_m_w,
                                                               self->start_m_h,
                                                               tbo_frame_get_x (self->selected_frame),
                                                               tbo_frame_get_y (self->selected_frame),
                                                                tbo_frame_get_width (self->selected_frame),
                                                                tbo_frame_get_height (self->selected_frame)));
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
    }

    should_open_text_editor = self->edit_text_on_release &&
                              self->selected_object != NULL &&
                              TBO_IS_OBJECT_TEXT (self->selected_object) &&
                              !moved_object (self) &&
                              !self->resizing &&
                              !self->rotating;

    self->start_x = 0;
    self->start_y = 0;
    self->clicked = FALSE;
    self->edit_text_on_release = FALSE;
    self->resizing = FALSE;
    self->rotating = FALSE;

    if (should_open_text_editor)
        open_text_editor (self, TBO_OBJECT_TEXT (self->selected_object));
}

static void
on_key (TboToolBase *tool, GtkWidget *widget, TboKeyEvent event)
{
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);
    Frame *frame = tbo_drawing_get_current_frame (drawing);

    if (event.keyval == GDK_KEY_Delete || event.keyval == GDK_KEY_KP_Delete)
    {
        if (delete_selected (self))
            tbo_drawing_update (drawing);
        return;
    }

    if (frame)
        frame_view_on_key (tool, widget, event);
    else if (self->selected_frame != NULL &&
             (event.keyval == GDK_KEY_Return || event.keyval == GDK_KEY_KP_Enter))
        tbo_window_enter_frame (tool->tbo, self->selected_frame);
}

static gboolean
delete_selected (TboToolSelector *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    TboDrawing *drawing = TBO_DRAWING (tbo->drawing);
    TboObjectBase *obj = self->selected_object;
    Frame *frame = self->selected_frame;
    Page *page = tbo_comic_get_current_page (tbo->comic);

    if (obj != NULL && tbo_drawing_get_current_frame (drawing) != NULL)
    {
        gint index = tbo_frame_object_nth (frame, obj);
        TboAction *action = tbo_action_object_remove_new (frame, obj, index);

        tbo_tool_selector_set_selected_object_pointer (self, NULL);
        tbo_frame_del_obj (frame, obj);
        tbo_undo_stack_insert (tbo->undo_stack, action);
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
        update_menubar (tbo);
        return TRUE;
    }

    if (frame != NULL && tbo_drawing_get_current_frame (drawing) == NULL)
    {
        gint index = tbo_page_frame_nth (page, frame);
        TboAction *action = tbo_action_frame_remove_new (page, frame, index);

        tbo_page_del_frame (page, frame);
        tbo_undo_stack_insert (tbo->undo_stack, action);
        tbo_tool_selector_set_selected (self, NULL);
        tbo_window_mark_dirty (tbo);
        tbo_window_refresh_status (tbo);
        tbo_toolbar_update (tbo->toolbar);
        return TRUE;
    }

    return FALSE;
}

static void
open_text_editor (TboToolSelector *self, TboObjectText *text)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    TboToolText *text_tool;

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_TEXT);
    text_tool = TBO_TOOL_TEXT (tbo->toolbar->tools[TBO_TOOLBAR_TEXT]);
    tbo_tool_text_set_selected (text_tool, text);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
}

static void
drawing (TboToolBase *tool, cairo_t *cr)
{
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);
    Frame *frame = tbo_drawing_get_current_frame (drawing);
    if (frame)
        frame_view_drawing (tool, cr);
    else
        page_view_drawing (tool, cr);
}

/* frame view */
static void
frame_view_on_move (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event)
{
    int x, y, offset_x, offset_y;
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);

    x = (int)event->x;
    y = (int)event->y;

    self->x = x;
    self->y = y;

    if (self->selected_object != NULL)
    {
        if (self->clicked)
        {
            gdouble scale = tbo_drawing_get_current_frame_scale (drawing);

            offset_x = (self->start_x - x) / scale;
            offset_y = (self->start_y - y) / scale;

            // resizing object
            if (self->resizing)
            {
                self->selected_object->width = clamp_object_dimension (self->start_m_w - offset_x);
                self->selected_object->height = clamp_object_dimension (self->start_m_h - offset_y);
            }
            else if (self->rotating)
            {
                self->selected_object->angle = atan2 (offset_y, offset_x);
            }
            // moving object
            else
            {
                self->selected_object->x = self->start_m_x - offset_x;
                self->selected_object->y = self->start_m_y - offset_y;
            }
        }

        // updating group object
        if (TBO_IS_OBJECT_GROUP (self->selected_object))
        {
            tbo_object_group_update_status (TBO_OBJECT_GROUP (self->selected_object));

            self->start_x = x;
            self->start_y = y;
            self->start_m_x = self->selected_object->x;
            self->start_m_y = self->selected_object->y;
            self->start_m_w = self->selected_object->width;
            self->start_m_h = self->selected_object->height;
            self->start_m_angle = self->selected_object->angle;
        }

        tbo_object_group_set_vars (self->selected_object);
        // over resizer
        if (over_resizer_obj (self, self->selected_object, x, y))
        {
            self->over_resizer = TRUE;
        }
        else
        {
            self->over_resizer = FALSE;
        }
        // over rotater
        if (over_rotater_obj (self, self->selected_object, x, y))
        {
            self->over_rotater = TRUE;
        }
        else
        {
            self->over_rotater = FALSE;
        }
        tbo_object_group_unset_vars (self->selected_object);

    }
}

static void
frame_view_on_click (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event)
{
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    int x, y;
    GList *obj_list;
    Frame *frame;
    TboObjectBase *obj, *obj2;
    TboObjectGroup *group;
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);
    gboolean found = FALSE;

    x = (int)event->x;
    y = (int)event->y;
    self->edit_text_on_release = FALSE;

    // resizing
    tbo_object_group_set_vars (self->selected_object);
    if (self->selected_object && over_resizer_obj (self, self->selected_object, x, y))
    {
        self->resizing = TRUE;
    }
    else if (self->selected_object && over_rotater_obj (self, self->selected_object, x, y))
    {
        self->rotating = TRUE;
    }
    else
    {
        frame = tbo_drawing_get_current_frame (drawing);

        for (obj_list = tbo_frame_get_objects (frame); obj_list; obj_list = obj_list->next)
        {
            obj = TBO_OBJECT_BASE (obj_list->data);
            tbo_object_group_set_vars (obj);
            if (tbo_drawing_point_inside_object (drawing, obj, x, y))
            {
                // Selecting last occurrence.
                obj2 = obj;
                found = TRUE;
            }
            tbo_object_group_unset_vars (obj);
        }

        if (!found)
            tbo_tool_selector_set_selected_obj (self, NULL);
        else
        {
            if ((event->state & GDK_SHIFT_MASK) && self->selected_object) {
                if (!TBO_IS_OBJECT_GROUP (self->selected_object))
                {
                    group = TBO_OBJECT_GROUP (tbo_object_group_new ());
                    tbo_frame_add_obj (frame, TBO_OBJECT_BASE (group));
                    tbo_object_group_add (group, self->selected_object);
                }
                else
                    group = TBO_OBJECT_GROUP (self->selected_object);

                tbo_object_group_add (group, obj2);
                obj2 = TBO_OBJECT_BASE (group);
            }

            self->edit_text_on_release = (obj2 == self->selected_object) &&
                                         TBO_IS_OBJECT_TEXT (obj2) &&
                                         !(event->state & GDK_SHIFT_MASK);
            tbo_tool_selector_set_selected_obj (self, obj2);
        }
    }
    tbo_object_group_unset_vars (self->selected_object);

    self->start_x = x;
    self->start_y = y;

    if (self->selected_object)
    {
        self->start_m_x = self->selected_object->x;
        self->start_m_y = self->selected_object->y;
        self->start_m_w = self->selected_object->width;
        self->start_m_h = self->selected_object->height;
        self->start_m_angle = self->selected_object->angle;
    }
    self->clicked = TRUE;
}

static void
frame_view_drawing (TboToolBase *tool, cairo_t *cr)
{
    const double dashes[] = {5, 5};
    Color border = {0.9, 0.9, 0};
    Color white = {1, 1, 1};
    Color black = {0, 0, 0};
    Color *resizer_border;
    Color *resizer_fill;
    Color *rotater_border;
    Color *rotater_fill;
    int x, y;
    float r_size;
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    TboObjectBase *current_obj = self->selected_object;
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);
    Frame *frame = tbo_drawing_get_current_frame (drawing);

    if (current_obj != NULL && !G_IS_OBJECT (current_obj))
    {
        tbo_tool_selector_set_selected_object_pointer (self, NULL);
        current_obj = NULL;
    }

    if (current_obj != NULL && frame != NULL && !tbo_frame_has_obj (frame, current_obj))
    {
        tbo_tool_selector_set_selected_object_pointer (self, NULL);
        current_obj = NULL;
    }

    if (current_obj != NULL)
    {
        if (TBO_IS_OBJECT_GROUP (current_obj))
            tbo_object_group_set_vars (current_obj);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, G_N_ELEMENTS (dashes), 0);
        cairo_set_source_rgb (cr, border.r, border.g, border.b);
        int ox, oy, ow, oh;
        tbo_drawing_get_object_relative (drawing, current_obj, &ox, &oy, &ow, &oh);

        cairo_translate (cr, ox, oy);
        cairo_rotate (cr, current_obj->angle);
        cairo_rectangle (cr, 0, 0, ow, oh);
        cairo_stroke (cr);

        // resizer
        if (self->over_resizer)
        {
            resizer_fill = &black;
            resizer_border = &white;
        }
        else
        {
            resizer_fill = &white;
            resizer_border = &black;
        }

        // rotater
        if (self->over_rotater)
        {
            rotater_fill = &black;
            rotater_border = &white;
        }
        else
        {
            rotater_fill = &white;
            rotater_border = &black;
        }

        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, 0, 0);

        x = ow;
        y = oh;

        r_size = R_SIZE / tbo_drawing_get_zoom (drawing);
        cairo_set_line_width (cr, 1/tbo_drawing_get_zoom (drawing));

        cairo_rectangle (cr, x, y, r_size, r_size);
        cairo_set_source_rgb(cr, resizer_fill->r, resizer_fill->g, resizer_fill->b);
        cairo_fill (cr);

        cairo_set_source_rgb(cr, resizer_border->r, resizer_border->g, resizer_border->b);
        cairo_rectangle (cr, x, y, r_size, r_size);
        cairo_stroke (cr);

        // object rotate zone
        cairo_set_source_rgb(cr, rotater_fill->r, rotater_fill->g, rotater_fill->b);
        cairo_arc (cr, 0, 0, r_size / 2., 0, 2 * M_PI);
        cairo_fill (cr);
        cairo_set_source_rgb(cr, rotater_border->r, rotater_border->g, rotater_border->b);
        cairo_arc (cr, 0, 0, r_size / 2., 0, 2 * M_PI);
        cairo_stroke (cr);
        cairo_set_line_width (cr, tbo_drawing_get_zoom (drawing));

        cairo_rotate (cr, -current_obj->angle);
        cairo_translate (cr, -ox, -oy);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);

        if (self->rotating)
        {
            cairo_set_source_rgb(cr, 1, 0, 0);
            cairo_move_to (cr, ox, oy);
            cairo_line_to (cr, self->x, self->y);
            cairo_stroke (cr);
        }

        if (TBO_IS_OBJECT_GROUP (current_obj))
            tbo_object_group_unset_vars (current_obj);
    }
}

static void
frame_view_on_key (TboToolBase *tool, GtkWidget *widget, TboKeyEvent event)
{
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    TboObjectBase *current_obj = self->selected_object;
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);

    if (self->selected_frame != NULL && event.keyval == GDK_KEY_Escape)
    {
        tbo_window_leave_frame (tool->tbo);
    }

    if (current_obj != NULL)
    {
        int x1 = current_obj->x;
        int y1 = current_obj->y;
        int width1 = current_obj->width;
        int height1 = current_obj->height;
        gdouble angle1 = current_obj->angle;

        switch (event.keyval)
        {
            case GDK_KEY_less:
                tbo_object_base_resize (current_obj, RESIZE_LESS);
                break;
            case GDK_KEY_greater:
                tbo_object_base_resize (current_obj, RESIZE_GREATER);
                break;
            case GDK_KEY_Up:
                tbo_object_base_move (current_obj, MOVE_UP);
                break;
            case GDK_KEY_Down:
                tbo_object_base_move (current_obj, MOVE_DOWN);
                break;
            case GDK_KEY_Left:
                tbo_object_base_move (current_obj, MOVE_LEFT);
                break;
            case GDK_KEY_Right:
                tbo_object_base_move (current_obj, MOVE_RIGHT);
                break;
            default:
                break;
        }

        if (x1 != current_obj->x ||
            y1 != current_obj->y ||
            width1 != current_obj->width ||
            height1 != current_obj->height ||
            fabs (angle1 - current_obj->angle) > ANGLE_EPSILON)
        {
            tbo_undo_stack_insert (tool->tbo->undo_stack,
                                   tbo_action_object_transform_new (current_obj,
                                                                    x1,
                                                                    y1,
                                                                    width1,
                                                                    height1,
                                                                    angle1,
                                                                    current_obj->x,
                                                                    current_obj->y,
                                                                    current_obj->width,
                                                                    current_obj->height,
                                                                    current_obj->angle));
            tbo_window_mark_dirty (tool->tbo);
            tbo_toolbar_update (tool->tbo->toolbar);
        }
    }
    tbo_drawing_update (drawing);
}

static void
page_view_drawing (TboToolBase *tool, cairo_t *cr)
{
    const double dashes[] = {5, 5};
    Color border = {0.9, 0.9, 0};
    Color white = {1, 1, 1};
    Color black = {0, 0, 0};
    Color *resizer_border;
    Color *resizer_fill;
    int x, y;
    float r_size;

    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    Frame *selected = self->selected_frame;
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);

    if (selected != NULL)
    {
        int selected_x = tbo_frame_get_x (selected);
        int selected_y = tbo_frame_get_y (selected);
        int selected_width = tbo_frame_get_width (selected);
        int selected_height = tbo_frame_get_height (selected);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, G_N_ELEMENTS (dashes), 0);
        cairo_set_source_rgb (cr, border.r, border.g, border.b);
        cairo_rectangle (cr, selected_x, selected_y, selected_width, selected_height);
        cairo_stroke (cr);

        // resizer
        if (self->over_resizer)
        {
            resizer_fill = &black;
            resizer_border = &white;
        }
        else
        {
            resizer_fill = &white;
            resizer_border = &black;
        }

        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, 0, 0);

        x = selected_x + selected_width;
        y = selected_y + selected_height;

        r_size = R_SIZE / tbo_drawing_get_zoom (drawing);
        cairo_set_line_width (cr, 1 / tbo_drawing_get_zoom (drawing));
        cairo_rectangle (cr, x, y, r_size, r_size);
        cairo_set_source_rgb(cr, resizer_fill->r, resizer_fill->g, resizer_fill->b);
        cairo_fill (cr);

        cairo_set_source_rgb(cr, resizer_border->r, resizer_border->g, resizer_border->b);
        cairo_rectangle (cr, x, y, r_size, r_size);
        cairo_stroke (cr);
        cairo_set_line_width (cr, tbo_drawing_get_zoom (drawing));

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
    }
}

static void
page_view_on_click (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event)
{
    int x, y;
    GList *frame_list;
    Page *page;
    Frame *frame;
    gboolean found = FALSE;

    TboWindow *tbo = tool->tbo;
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    Frame *selected;

    x = (int)event->x;
    y = (int)event->y;


    page = tbo_comic_get_current_page (tbo->comic);
    for (frame_list = tbo_page_get_frames (page); frame_list; frame_list = frame_list->next)
    {
        frame = (Frame *)frame_list->data;
        if (tbo_frame_point_inside (frame, x, y))
        {
            // Selecting last occurrence.
            tbo_tool_selector_set_selected (self, frame);
            found = TRUE;
        }
    }
    selected = self->selected_frame;

    // resizing
    if (selected && over_resizer (self, selected, x, y))
    {
        self->resizing = TRUE;
    }
    else if (!found)
        tbo_tool_selector_set_selected (self, NULL);

    // double click, frame view
    if (selected && event->n_press == 2)
    {
        self->clicked = FALSE;
        self->resizing = FALSE;
        tbo_window_enter_frame (tbo, selected);
        return;
    }

    self->start_x = x;
    self->start_y = y;

    if (selected)
    {
        self->start_m_x = tbo_frame_get_x (selected);
        self->start_m_y = tbo_frame_get_y (selected);
        self->start_m_w = tbo_frame_get_width (selected);
        self->start_m_h = tbo_frame_get_height (selected);
        tbo_page_set_current_frame (page, selected);
    }
    self->clicked = TRUE;
}

static void
page_view_on_move (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event)
{
    int x, y, offset_x, offset_y;
    TboWindow *tbo = tool->tbo;
    TboToolSelector *self = TBO_TOOL_SELECTOR (tool);
    Frame *selected = self->selected_frame;

    x = (int)event->x;
    y = (int)event->y;

    if (selected != NULL)
    {
        if (self->clicked)
        {
            offset_x = (self->start_x - x);
            offset_y = (self->start_y - y);

            // resizing frame
            if (self->resizing)
            {
                tbo_frame_set_size (selected,
                                    clamp_frame_dimension (abs (self->start_m_w - offset_x)),
                                    clamp_frame_dimension (abs (self->start_m_h - offset_y)));

                update_tool_area (self);
            }
            // moving frame
            else
            {
                tbo_frame_set_position (selected,
                                        self->start_m_x - offset_x,
                                        self->start_m_y - offset_y);

                update_tool_area (self);
            }
        }

        // over resizer
        if (over_resizer (self, selected, x, y))
        {
            self->over_resizer = TRUE;
        }
        else
        {
            self->over_resizer = FALSE;
        }
    }

    GList *frame_list;
    Frame *frame;
    Page *page = tbo_comic_get_current_page (tbo->comic);
    gboolean found = FALSE;
    int x1, y1;

    for (frame_list = tbo_page_get_frames (page); frame_list && !found; frame_list = frame_list->next)
    {
        if (tbo_frame_point_inside ((Frame*)frame_list->data, x, y))
        {
            frame = (Frame*)frame_list->data;
            x1 = tbo_frame_get_x (frame) + (tbo_frame_get_width (frame) / 2);
            y1 = tbo_frame_get_y (frame) + (tbo_frame_get_height (frame) / 2);
            tbo_tooltip_set (_("double click or press Enter"), x1, y1, tbo);
            found = TRUE;
        }
    }
    if (!found)
        tbo_tooltip_set(NULL, 0, 0, tbo);
}

/* init methods */

static void
tbo_tool_selector_init (TboToolSelector *self)
{
    self->x = 0;
    self->y = 0;
    self->start_x = 0;
    self->start_y = 0;
    self->start_m_x = 0;
    self->start_m_y = 0;
    self->start_m_w = 0;
    self->start_m_h = 0;
    self->start_m_angle = 0.0;
    self->clicked = FALSE;
    self->edit_text_on_release = FALSE;
    self->over_resizer = FALSE;
    self->over_rotater = FALSE;
    self->resizing = FALSE;
    self->rotating = FALSE;
    self->toolarea_widget = NULL;
    self->spin_w = NULL;
    self->spin_h = NULL;
    self->spin_x = NULL;
    self->spin_y = NULL;
    self->color_button = NULL;
    self->border_button = NULL;

    self->parent_instance.on_select = on_select;
    self->parent_instance.on_unselect = on_unselect;
    self->parent_instance.on_move = on_move;
    self->parent_instance.on_click = on_click;
    self->parent_instance.on_release = on_release;
    self->parent_instance.on_key = on_key;
    self->parent_instance.drawing = drawing;
}

static void
tbo_tool_selector_class_init (TboToolSelectorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = finalize;
}

static void
finalize (GObject *object)
{
    TboToolSelector *self = TBO_TOOL_SELECTOR (object);

    clear_selected_group (self);
    tbo_tool_selector_set_selected_frame_pointer (self, NULL);
    tbo_tool_selector_set_selected_object_pointer (self, NULL);

    if (self->toolarea_widget != NULL)
        g_object_unref (self->toolarea_widget);

    G_OBJECT_CLASS (tbo_tool_selector_parent_class)->finalize (object);
}

/* object functions */

GObject *
tbo_tool_selector_new (void)
{
    GObject *tbo_tool;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_SELECTOR, NULL);
    return tbo_tool;
}

GObject *
tbo_tool_selector_new_with_params (TboWindow *tbo)
{
    GObject *tbo_tool;
    TboToolBase *tbo_tool_base;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_SELECTOR, NULL);
    tbo_tool_base = TBO_TOOL_BASE (tbo_tool);
    tbo_tool_base->tbo = tbo;
    return tbo_tool;
}

Frame *
tbo_tool_selector_get_selected_frame (TboToolSelector *self)
{
    return self->selected_frame;
}

TboObjectBase *
tbo_tool_selector_get_selected_obj (TboToolSelector *self)
{
    return self->selected_object;
}

void
tbo_tool_selector_set_selected (TboToolSelector *self, Frame *frame)
{
    if (self->selected_frame == frame)
    {
        update_menubar (TBO_TOOL_BASE (self)->tbo);
        return;
    }

    tbo_tool_selector_set_selected_frame_pointer (self, frame);
    if (self->selected_frame != NULL)
        update_tool_area (self);
    update_menubar (TBO_TOOL_BASE (self)->tbo);
}

void
tbo_tool_selector_set_selected_obj (TboToolSelector *self, TboObjectBase *obj)
{
    if (obj != self->selected_object)
        clear_selected_group (self);

    tbo_tool_selector_set_selected_object_pointer (self, obj);
    update_menubar (TBO_TOOL_BASE (self)->tbo);
}

gboolean
tbo_tool_selector_delete_selected (TboToolSelector *self)
{
    return delete_selected (self);
}

void
tbo_tool_selector_reset_state (TboToolSelector *self)
{
    if (self == NULL)
        return;

    clear_selected_group (self);
    tbo_tool_selector_set_selected_frame_pointer (self, NULL);
    tbo_tool_selector_set_selected_object_pointer (self, NULL);
    self->x = 0;
    self->y = 0;
    self->start_x = 0;
    self->start_y = 0;
    self->start_m_x = 0;
    self->start_m_y = 0;
    self->start_m_w = 0;
    self->start_m_h = 0;
    self->start_m_angle = 0.0;
    self->clicked = FALSE;
    self->edit_text_on_release = FALSE;
    self->over_resizer = FALSE;
    self->over_rotater = FALSE;
    self->resizing = FALSE;
    self->rotating = FALSE;
}
