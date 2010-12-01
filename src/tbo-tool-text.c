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
#include "tbo-drawing.h"
#include "tbo-object-base.h"
#include "tbo-tool-text.h"

G_DEFINE_TYPE (TboToolText, tbo_tool_text, TBO_TYPE_TOOL_BASE);

#define DEFAULT_PANGO_FONT "Sans Normal 27"

/* Headers */
static void on_select (TboToolBase *tool);
static void on_unselect (TboToolBase *tool);
static void on_click (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event);
static void drawing (TboToolBase *tool, cairo_t *cr);

/* Definitions */

/* aux */
static gboolean
on_tview_focus_in (GtkWidget *view, GdkEventFocus *event, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    tbo_window_set_key_binder (tbo, FALSE);
    return FALSE;
}

static gboolean
on_tview_focus_out (GtkWidget *view, GdkEventFocus *event, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    tbo_window_set_key_binder (tbo, TRUE);
    return FALSE;
}


static gboolean
on_text_change (GtkTextBuffer *buf, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (buf, &start);
    gtk_text_buffer_get_end_iter (buf, &end);

    if (self->text_selected)
    {
        tbo_object_text_set_text (self->text_selected, gtk_text_buffer_get_text (buf, &start, &end, FALSE));
        tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    }
    return FALSE;
}

static gboolean
on_font_change (GtkFontButton *fbutton, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    if (self->text_selected)
    {
        tbo_object_text_change_font (self->text_selected, tbo_tool_text_get_pango_font (self));
        tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    }
    return FALSE;
}

static gboolean
on_color_change (GtkColorButton *cbutton, TboToolText *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    if (self->text_selected)
    {
        GdkColor color;
        gtk_color_button_get_color (GTK_COLOR_BUTTON (self->font_color), &color);
        tbo_object_text_change_color (self->text_selected, &color);
        tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    }
    return FALSE;
}

GtkWidget *
setup_toolarea (TboToolText *self)
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *font_color_label = gtk_label_new (_("Text color:"));
    GtkWidget *font_label = gtk_label_new (_("Font:"));
    GtkWidget *scroll;
    GtkWidget *view;

    gtk_misc_set_alignment (GTK_MISC (font_label), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (font_color_label), 0, 0);

    self->font = gtk_font_button_new ();
    g_signal_connect (self->font, "font-set", G_CALLBACK (on_font_change), self);
    self->font_color = gtk_color_button_new ();
    g_signal_connect (self->font_color, "color-set", G_CALLBACK (on_color_change), self);

    vbox = gtk_vbox_new (FALSE, 5);

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), font_label, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), self->font, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), font_color_label, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), self->font_color, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);

    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    view = gtk_text_view_new ();
    gtk_widget_add_events (view, GDK_FOCUS_CHANGE_MASK);
    g_signal_connect (view, "focus-in-event", G_CALLBACK (on_tview_focus_in), self);
    g_signal_connect (view, "focus-out-event", G_CALLBACK (on_tview_focus_out), self);

    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
    self->text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_set_text (self->text_buffer, "", -1);
    g_signal_connect (self->text_buffer, "changed", G_CALLBACK (on_text_change), self);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), view);
    gtk_box_pack_start (GTK_BOX (vbox), scroll, FALSE, FALSE, 5);

    return vbox;
}

/* tool signal */
static void
on_select (TboToolBase *tool)
{
    GtkWidget *toolarea = setup_toolarea (TBO_TOOL_TEXT (tool));
    gtk_widget_show_all (GTK_WIDGET (toolarea));
    tbo_empty_tool_area (tool->tbo);
    gtk_container_add (GTK_CONTAINER (tool->tbo->toolarea), toolarea);
}

static void
on_unselect (TboToolBase *tool)
{
    /* TODO remove widgets from toolarea to not destroy it */
    tbo_empty_tool_area (tool->tbo);
    tbo_window_set_key_binder (tool->tbo, TRUE);
}

static void
on_click (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event)
{
    int x = (int)event->x;
    int y = (int)event->y;
    gboolean found = FALSE;
    GList *obj_list;
    TboObjectBase *obj;
    TboObjectText *text;
    GdkColor color;
    TboToolText *self = TBO_TOOL_TEXT (tool);
    Frame *frame = tbo_drawing_get_current_frame (TBO_DRAWING (tool->tbo->drawing));

    for (obj_list = g_list_first (frame->objects); obj_list; obj_list = obj_list->next)
    {
        obj = TBO_OBJECT_BASE (obj_list->data);
        if (TBO_IS_OBJECT_TEXT (obj) && tbo_frame_point_inside_obj (obj, x, y))
        {
            text = TBO_OBJECT_TEXT (obj);
            found = TRUE;
        }
    }
    if (!found)
    {
        x = tbo_frame_get_base_x (x);
        y = tbo_frame_get_base_y (y);
        gtk_color_button_get_color (GTK_COLOR_BUTTON (self->font_color), &color);
        text = TBO_OBJECT_TEXT (tbo_object_text_new_with_params (x, y, 100, 0,
                                                _("Text"),
                                                tbo_tool_text_get_pango_font (self),
                                                &color));
        tbo_frame_add_obj (frame, TBO_OBJECT_BASE (text));
    }
    tbo_tool_text_set_selected (self, text);
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
        tbo_frame_get_obj_relative (obj, &ox, &oy, &ow, &oh);

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
    self->font_color = NULL;
    self->text_selected = NULL;
    self->text_buffer = NULL;

    self->parent_instance.on_select = on_select;
    self->parent_instance.on_unselect = on_unselect;
    self->parent_instance.on_click = on_click;
    self->parent_instance.drawing = drawing;
}

static void
tbo_tool_text_class_init (TboToolTextClass *klass)
{
}

/* object functions */

GObject *
tbo_tool_text_new ()
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
    if (self->font)
    {
        return (gchar *)gtk_font_button_get_font_name (GTK_FONT_BUTTON (self->font));
    }

    return DEFAULT_PANGO_FONT;
}
gchar *
tbo_tool_text_get_font_name (TboToolText *self)
{
    PangoFontDescription *pango_font = NULL;

    if (self->font)
    {
        pango_font = pango_font_description_from_string (
                gtk_font_button_get_font_name (GTK_FONT_BUTTON (self->font)));
        return (gchar *)pango_font_description_get_family (pango_font);
    }

    return NULL;
}

void
tbo_tool_text_set_selected (TboToolText *self, TboObjectText *text)
{
    char *str = tbo_object_text_get_text (text);
    self->text_selected = NULL;
    gtk_font_button_set_font_name (GTK_FONT_BUTTON (self->font), tbo_object_text_get_string (text));
    gtk_color_button_set_color (GTK_COLOR_BUTTON (self->font_color), text->font_color);
    gtk_text_buffer_set_text (self->text_buffer, str, -1);
    self->text_selected = text;
}

