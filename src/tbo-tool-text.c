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

#include <glib/gi18n.h>
#include "frame.h"
#include "tbo-window.h"
#include "tbo-widget.h"
#include "tbo-drawing.h"
#include "tbo-ui-utils.h"
#include "tbo-object-base.h"
#include "tbo-tool-selector.h"
#include "tbo-tool-text.h"
#include "tbo-undo.h"

G_DEFINE_TYPE (TboToolText, tbo_tool_text, TBO_TYPE_TOOL_BASE);

#define DEFAULT_PANGO_FONT "Sans Normal 27"

/* Headers */
static void on_select (TboToolBase *tool);
static void on_unselect (TboToolBase *tool);
static void on_click (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event);
static void drawing (TboToolBase *tool, cairo_t *cr);
static void finalize (GObject *object);

static gint tbo_tool_text_get_font_size (TboToolText *self);
static gchar *tbo_tool_text_build_font (TboToolText *self);
static void tbo_tool_text_sync_font_controls (TboToolText *self, const gchar *font_string);
static void on_text_begin_user_action (GtkTextBuffer *buf, TboToolText *self);
static void on_text_end_user_action (GtkTextBuffer *buf, TboToolText *self);
static void tbo_tool_text_capture_state (TboToolText *self);
static void tbo_tool_text_clear_capture_state (TboToolText *self);
static gboolean flush_pending_text_change (gpointer data);
static void tbo_tool_text_focus_editor (TboToolText *self, gboolean select_all);

static void
tbo_tool_text_focus_editor (TboToolText *self, gboolean select_all)
{
    GtkTextIter start;
    GtkTextIter end;

    if (self->text_view == NULL || self->text_buffer == NULL)
        return;

    if (select_all)
    {
        gtk_text_buffer_get_bounds (self->text_buffer, &start, &end);
        gtk_text_buffer_select_range (self->text_buffer, &start, &end);
    }

    gtk_widget_grab_focus (self->text_view);
}

static void
tbo_tool_text_capture_state (TboToolText *self)
{
    if (self->text_selected == NULL || self->captured_text != NULL)
        return;

    self->captured_text = g_strdup (tbo_object_text_get_text (self->text_selected));
    self->captured_font = tbo_object_text_get_string (self->text_selected);
    self->captured_color = *self->text_selected->font_color;
}

static void
tbo_tool_text_clear_capture_state (TboToolText *self)
{
    g_clear_pointer (&self->captured_text, g_free);
    g_clear_pointer (&self->captured_font, g_free);
}

static gboolean
flush_pending_text_change (gpointer data)
{
    TboToolText *self = TBO_TOOL_TEXT (data);
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    gchar *text;
    gchar *font;
    GtkTextIter start, end;

    self->pending_text_change_id = 0;

    if (self->text_selected == NULL || self->captured_text == NULL)
    {
        tbo_tool_text_clear_capture_state (self);
        return G_SOURCE_REMOVE;
    }

    gtk_text_buffer_get_start_iter (self->text_buffer, &start);
    gtk_text_buffer_get_end_iter (self->text_buffer, &end);
    text = gtk_text_buffer_get_text (self->text_buffer, &start, &end, FALSE);
    font = tbo_object_text_get_string (self->text_selected);

    if (g_strcmp0 (self->captured_text, text) != 0)
    {
        tbo_undo_stack_insert (tbo->undo_stack,
                               tbo_action_text_state_new (self->text_selected,
                                                          self->captured_text,
                                                          self->captured_font,
                                                          &self->captured_color,
                                                          text,
                                                          font,
                                                          self->text_selected->font_color));
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
    }

    g_free (text);
    g_free (font);
    tbo_tool_text_clear_capture_state (self);
    return G_SOURCE_REMOVE;
}

/* Definitions */

/* aux */
static void
on_tview_focus_changed (GtkWidget *view, GParamSpec *pspec, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    tbo_window_set_key_binder (tbo, !gtk_widget_has_focus (view));
}


static gboolean
on_text_change (GtkTextBuffer *buf, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (buf, &start);
    gtk_text_buffer_get_end_iter (buf, &end);

    if (self->syncing_controls)
        return FALSE;

    if (self->text_selected)
    {
        gchar *old_text = g_strdup (tbo_object_text_get_text (self->text_selected));
        gchar *old_font = tbo_object_text_get_string (self->text_selected);
        gchar *text = gtk_text_buffer_get_text (buf, &start, &end, FALSE);

        if (g_strcmp0 (old_text, text) == 0)
        {
            g_free (old_text);
            g_free (old_font);
            g_free (text);
            return FALSE;
        }

        if (!self->text_capture_active)
            tbo_tool_text_capture_state (self);

        tbo_object_text_set_text (self->text_selected, text);
        if (!self->text_capture_active)
        {
            if (self->pending_text_change_id == 0)
                self->pending_text_change_id = g_idle_add (flush_pending_text_change, self);
        }
        g_free (old_text);
        g_free (old_font);
        g_free (text);
        tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    }
    return FALSE;
}

static void
on_text_begin_user_action (GtkTextBuffer *buf, TboToolText *self)
{
    (void) buf;

    if (self->syncing_controls || self->text_selected == NULL || self->text_capture_active)
        return;

    self->text_capture_active = TRUE;
    tbo_tool_text_capture_state (self);
}

static void
on_text_end_user_action (GtkTextBuffer *buf, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    GtkTextIter start, end;
    gchar *text;
    gchar *font;

    (void) buf;

    if (!self->text_capture_active || self->text_selected == NULL)
        return;

    if (self->pending_text_change_id != 0)
    {
        g_source_remove (self->pending_text_change_id);
        self->pending_text_change_id = 0;
    }

    gtk_text_buffer_get_start_iter (self->text_buffer, &start);
    gtk_text_buffer_get_end_iter (self->text_buffer, &end);
    text = gtk_text_buffer_get_text (self->text_buffer, &start, &end, FALSE);
    font = tbo_object_text_get_string (self->text_selected);

    if (g_strcmp0 (self->captured_text, text) != 0)
    {
        tbo_undo_stack_insert (tbo->undo_stack,
                               tbo_action_text_state_new (self->text_selected,
                                                          self->captured_text,
                                                          self->captured_font,
                                                          &self->captured_color,
                                                          text,
                                                          font,
                                                          self->text_selected->font_color));
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
    }

    g_free (text);
    g_free (font);
    tbo_tool_text_clear_capture_state (self);
    self->text_capture_active = FALSE;
}

static void
on_font_change (GtkWidget *widget, GParamSpec *pspec, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    if (self->syncing_controls)
        return;

    if (self->text_selected)
    {
        gchar *old_text = g_strdup (tbo_object_text_get_text (self->text_selected));
        gchar *old_font = tbo_object_text_get_string (self->text_selected);
        GdkRGBA old_color = *self->text_selected->font_color;
        gchar *font = tbo_tool_text_build_font (self);

        if (g_strcmp0 (old_font, font) == 0)
        {
            g_free (old_text);
            g_free (old_font);
            g_free (font);
            return;
        }

        tbo_object_text_change_font (self->text_selected, font);
        tbo_undo_stack_insert (tbo->undo_stack,
                               tbo_action_text_state_new (self->text_selected,
                                                          old_text,
                                                          old_font,
                                                          &old_color,
                                                          old_text,
                                                          font,
                                                          &old_color));
        g_free (old_text);
        g_free (old_font);
        g_free (font);
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
        tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    }
}

static gboolean
on_font_size_change (GtkSpinButton *spin, TboToolText *self)
{
    on_font_change (NULL, NULL, self);
    return FALSE;
}

static void
on_color_change (GtkWidget *widget, GParamSpec *pspec, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    if (self->syncing_controls)
        return;

    if (self->text_selected)
    {
        gchar *old_text = g_strdup (tbo_object_text_get_text (self->text_selected));
        gchar *old_font = tbo_object_text_get_string (self->text_selected);
        GdkRGBA old_color = *self->text_selected->font_color;
        GdkRGBA color = tbo_color_picker_get_rgba (self->font_color);

        if (gdk_rgba_equal (&old_color, &color))
        {
            g_free (old_text);
            g_free (old_font);
            return;
        }

        tbo_object_text_change_color (self->text_selected, &color);
        tbo_undo_stack_insert (tbo->undo_stack,
                               tbo_action_text_state_new (self->text_selected,
                                                          old_text,
                                                          old_font,
                                                          &old_color,
                                                          old_text,
                                                          old_font,
                                                          &color));
        g_free (old_text);
        g_free (old_font);
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
        tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    }
}

GtkWidget *
setup_toolarea (TboToolText *self)
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *font_color_label = gtk_label_new (_("Text color:"));
    GtkWidget *font_label = gtk_label_new (_("Font:"));
    GtkWidget *font_size_label = gtk_label_new (_("Size:"));
    GtkWidget *scroll;
    GtkWidget *view;
    GtkAdjustment *font_size_adjustment;
    GdkRGBA default_color = { 0, 0, 0, 1 };

    gtk_label_set_xalign (GTK_LABEL (font_label), 0.0);
    gtk_label_set_yalign (GTK_LABEL (font_label), 0.5);
    gtk_label_set_xalign (GTK_LABEL (font_size_label), 0.0);
    gtk_label_set_yalign (GTK_LABEL (font_size_label), 0.5);
    gtk_label_set_xalign (GTK_LABEL (font_color_label), 0.0);
    gtk_label_set_yalign (GTK_LABEL (font_color_label), 0.5);

    self->font = tbo_font_picker_new ();
    g_signal_connect (self->font, "notify::font-desc", G_CALLBACK (on_font_change), self);

    font_size_adjustment = gtk_adjustment_new (27, 1, 300, 1, 5, 0);
    self->font_size = gtk_spin_button_new (font_size_adjustment, 1, 0);
    gtk_editable_set_alignment (GTK_EDITABLE (self->font_size), 0.5);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (self->font_size), TRUE);
    g_signal_connect (self->font_size, "value-changed", G_CALLBACK (on_font_size_change), self);

    self->font_color = tbo_color_picker_new (&default_color);
    g_signal_connect (self->font_color, "notify::rgba", G_CALLBACK (on_color_change), self);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    tbo_box_pack_start (hbox, font_label, TRUE, TRUE, 5);
    tbo_box_pack_start (hbox, self->font, TRUE, TRUE, 5);
    tbo_box_pack_start (vbox, hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    tbo_box_pack_start (hbox, font_size_label, TRUE, TRUE, 5);
    tbo_box_pack_start (hbox, self->font_size, TRUE, TRUE, 5);
    tbo_box_pack_start (vbox, hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    tbo_box_pack_start (hbox, font_color_label, TRUE, TRUE, 5);
    tbo_box_pack_start (hbox, self->font_color, TRUE, TRUE, 5);
    tbo_box_pack_start (vbox, hbox, FALSE, FALSE, 5);

    tbo_tool_text_sync_font_controls (self, DEFAULT_PANGO_FONT);

    scroll = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    view = gtk_text_view_new ();
    self->text_view = view;
    g_signal_connect (view, "notify::has-focus", G_CALLBACK (on_tview_focus_changed), self);

    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
    self->text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_set_text (self->text_buffer, "", -1);
    g_signal_connect (self->text_buffer, "changed", G_CALLBACK (on_text_change), self);
    g_signal_connect (self->text_buffer, "begin-user-action", G_CALLBACK (on_text_begin_user_action), self);
    g_signal_connect (self->text_buffer, "end-user-action", G_CALLBACK (on_text_end_user_action), self);
    tbo_scrolled_window_set_child (scroll, view);
    tbo_box_pack_start (vbox, scroll, FALSE, FALSE, 5);

    return vbox;
}

/* tool signal */
static void
on_select (TboToolBase *tool)
{
    GtkWidget *toolarea = setup_toolarea (TBO_TOOL_TEXT (tool));
    tbo_widget_show_all (toolarea);
    tbo_empty_tool_area (tool->tbo);
    tbo_widget_add_child (tool->tbo->toolarea, toolarea);
}

static void
on_unselect (TboToolBase *tool)
{
    TboToolText *self = TBO_TOOL_TEXT (tool);

    tbo_tool_text_set_selected (self, NULL);
    tbo_empty_tool_area (tool->tbo);
    self->text_view = NULL;
    self->text_buffer = NULL;
    tbo_window_set_key_binder (tool->tbo, TRUE);
}

static void
on_click (TboToolBase *tool, GtkWidget *widget, TboPointerEvent *event)
{
    int x = (int)event->x;
    int y = (int)event->y;
    gboolean found = FALSE;
    GList *obj_list;
    TboObjectBase *obj;
    TboObjectText *text;
    GdkRGBA color;
    TboToolText *self = TBO_TOOL_TEXT (tool);
    TboDrawing *drawing = TBO_DRAWING (tool->tbo->drawing);
    Frame *frame = tbo_drawing_get_current_frame (drawing);

    if (self->text_selected != NULL)
    {
        TboObjectText *current_text = g_object_ref (self->text_selected);
        TboToolSelector *selector;

        tbo_toolbar_set_selected_tool (tool->tbo->toolbar, TBO_TOOLBAR_SELECTOR);
        selector = TBO_TOOL_SELECTOR (tool->tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
        tbo_tool_selector_set_selected_obj (selector, TBO_OBJECT_BASE (current_text));
        tbo_drawing_update (TBO_DRAWING (tool->tbo->drawing));
        g_object_unref (current_text);
        return;
    }

    for (obj_list = tbo_frame_get_objects (frame); obj_list; obj_list = obj_list->next)
    {
        obj = TBO_OBJECT_BASE (obj_list->data);
        if (TBO_IS_OBJECT_TEXT (obj) && tbo_drawing_point_inside_object (drawing, obj, x, y))
        {
            text = TBO_OBJECT_TEXT (obj);
            found = TRUE;
        }
    }
    if (!found)
    {
        if (!tbo_drawing_view_to_frame (drawing, x, y, &x, &y))
            return;

        if (x < 0 || y < 0 || x > tbo_frame_get_width (frame) || y > tbo_frame_get_height (frame))
            return;

        gchar *font = tbo_tool_text_build_font (self);
        color = tbo_color_picker_get_rgba (self->font_color);
        text = TBO_OBJECT_TEXT (tbo_object_text_new_with_params (x, y, 100, 0,
                                                _("Text"),
                                                font,
                                                &color));
        g_free (font);
        tbo_frame_add_obj (frame, TBO_OBJECT_BASE (text));
        tbo_undo_stack_insert (tool->tbo->undo_stack,
                               tbo_action_object_add_new (frame, TBO_OBJECT_BASE (text)));
        tbo_window_mark_dirty (tool->tbo);
        tbo_toolbar_update (tool->tbo->toolbar);
    }
    tbo_tool_text_set_selected (self, text);
    if (!found)
        tbo_tool_text_focus_editor (self, TRUE);
    tbo_drawing_update (TBO_DRAWING (tool->tbo->drawing));
}

static void
drawing (TboToolBase *tool, cairo_t *cr)
{
    const double dashes[] = {5, 5};
    TboToolText *self = TBO_TOOL_TEXT (tool);

    if (self->text_selected)
    {
        TboObjectBase *obj = TBO_OBJECT_BASE (self->text_selected);
        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, G_N_ELEMENTS (dashes), 0);
        cairo_set_source_rgb (cr, 0.9, 0, 0);
        int ox, oy, ow, oh;
        tbo_drawing_get_object_relative (TBO_DRAWING (tool->tbo->drawing), obj, &ox, &oy, &ow, &oh);

        cairo_translate (cr, ox, oy);
        cairo_rotate (cr, obj->angle);
        cairo_rectangle (cr, 0, 0, ow, oh);
        cairo_stroke (cr);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
    }
}

/* init methods */

static void
tbo_tool_text_init (TboToolText *self)
{
    self->font = NULL;
    self->font_size = NULL;
    self->font_color = NULL;
    self->text_view = NULL;
    self->text_selected = NULL;
    self->text_buffer = NULL;
    self->syncing_controls = FALSE;
    self->text_capture_active = FALSE;
    self->pending_text_change_id = 0;
    self->captured_text = NULL;
    self->captured_font = NULL;

    self->parent_instance.on_select = on_select;
    self->parent_instance.on_unselect = on_unselect;
    self->parent_instance.on_click = on_click;
    self->parent_instance.drawing = drawing;
}

static void
tbo_tool_text_class_init (TboToolTextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = finalize;
}

static void
finalize (GObject *object)
{
    TboToolText *self = TBO_TOOL_TEXT (object);

    if (self->pending_text_change_id != 0)
        g_source_remove (self->pending_text_change_id);
    tbo_tool_text_clear_capture_state (self);
    tbo_tool_text_reset_state (TBO_TOOL_TEXT (object));
    G_OBJECT_CLASS (tbo_tool_text_parent_class)->finalize (object);
}

/* object functions */

GObject *
tbo_tool_text_new (void)
{
    GObject *tbo_tool;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_TEXT, NULL);
    return tbo_tool;
}

GObject *
tbo_tool_text_new_with_params (TboWindow *tbo)
{
    GObject *tbo_tool;
    TboToolBase *tbo_tool_base;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_TEXT, NULL);
    tbo_tool_base = TBO_TOOL_BASE (tbo_tool);
    tbo_tool_base->tbo = tbo;
    return tbo_tool;
}

gchar *
tbo_tool_text_get_pango_font (TboToolText *self)
{
    return tbo_tool_text_build_font (self);
}
gchar *
tbo_tool_text_get_font_name (TboToolText *self)
{
    PangoFontDescription *pango_font = NULL;

    if (self->font)
    {
        pango_font = tbo_font_picker_dup_font_desc (self->font);
        if (pango_font == NULL)
            return NULL;
        gchar *family = g_strdup (pango_font_description_get_family (pango_font));
        pango_font_description_free (pango_font);
        return family;
    }

    return NULL;
}

static gint
tbo_tool_text_get_font_size (TboToolText *self)
{
    if (self->font_size)
        return MAX (1, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (self->font_size)));

    return 27;
}

static gchar *
tbo_tool_text_build_font (TboToolText *self)
{
    PangoFontDescription *description = NULL;
    gchar *result;

    if (self->font)
        description = tbo_font_picker_dup_font_desc (self->font);

    if (description == NULL)
        description = pango_font_description_from_string (DEFAULT_PANGO_FONT);

    pango_font_description_set_size (description, tbo_tool_text_get_font_size (self) * PANGO_SCALE);
    result = pango_font_description_to_string (description);

    pango_font_description_free (description);
    return result;
}

static void
tbo_tool_text_sync_font_controls (TboToolText *self, const gchar *font_string)
{
    PangoFontDescription *description;
    gint size;

    if (self->font == NULL || self->font_size == NULL)
        return;

    description = pango_font_description_from_string (font_string);
    size = pango_font_description_get_size (description);
    if (size <= 0)
        size = 27;
    else
        size /= PANGO_SCALE;

    tbo_font_picker_set_font_desc (self->font, description);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->font_size), size);

    pango_font_description_free (description);
}

void
tbo_tool_text_set_selected (TboToolText *self, TboObjectText *text)
{
    char *str;

    if (self->pending_text_change_id != 0)
    {
        g_source_remove (self->pending_text_change_id);
        self->pending_text_change_id = 0;
    }
    self->text_capture_active = FALSE;
    tbo_tool_text_clear_capture_state (self);
    self->syncing_controls = TRUE;

    if (self->text_selected) {
        g_object_unref (self->text_selected);
        self->text_selected = NULL;
    }

    if (!text) {
        if (self->text_buffer)
            gtk_text_buffer_set_text (self->text_buffer, "", -1);
        if (self->font)
            tbo_tool_text_sync_font_controls (self, DEFAULT_PANGO_FONT);
        if (self->font_color)
        {
            GdkRGBA default_color = { 0, 0, 0, 1 };
            tbo_color_picker_set_rgba (self->font_color, &default_color);
        }
        self->syncing_controls = FALSE;
        return;
    }

    str = tbo_object_text_get_text (text);

    gchar *font = tbo_object_text_get_string (text);
    tbo_tool_text_sync_font_controls (self, font);
    g_free (font);
    tbo_color_picker_set_rgba (self->font_color, text->font_color);
    gtk_text_buffer_set_text (self->text_buffer, str, -1);
    self->text_selected = g_object_ref (text);
    self->syncing_controls = FALSE;
}

void
tbo_tool_text_reset_state (TboToolText *self)
{
    if (self == NULL)
        return;

    if (self->pending_text_change_id != 0)
    {
        g_source_remove (self->pending_text_change_id);
        self->pending_text_change_id = 0;
    }

    self->text_capture_active = FALSE;
    tbo_tool_text_clear_capture_state (self);

    if (self->text_selected == NULL)
        return;

    g_object_unref (self->text_selected);
    self->text_selected = NULL;
}
