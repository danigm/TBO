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
#include <glib/gi18n.h>

#include "tbo-window.h"
#include "tbo-toolbar.h"
#include "comic.h"
#include "tbo-object-pixmap.h"
#include "comic-new-dialog.h"
#include "comic-open-dialog.h"
#include "comic-saveas-dialog.h"
#include "tbo-file-dialog.h"
#include "tbo-drawing.h"
#include "dnd.h"
#include "frame.h"
#include "tbo-tool-selector.h"
#include "tbo-tool-frame.h"
#include "tbo-tool-doodle.h"
#include "tbo-tool-bubble.h"
#include "tbo-tool-text.h"
#include "tbo-ui-utils.h"
#include "ui-menu.h"
#include "tbo-undo.h"
#include "tbo-widget.h"
#include "tbo-utils.h"

G_DEFINE_TYPE (TboToolbar, tbo_toolbar, G_TYPE_OBJECT);

static void on_tool_button_toggled (GtkToggleButton *button, TboToolbar *toolbar);

static GtkWidget *
create_icon_wrapper (GtkWidget *child)
{
    GtkWidget *wrapper = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_size_request (wrapper, 20, 20);
    gtk_widget_set_halign (wrapper, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (wrapper, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class (wrapper, "tbo-toolbar-icon");
    tbo_widget_add_child (wrapper, child);
    return wrapper;
}

static GtkWidget *
create_icon_from_name (const gchar *icon_name)
{
    GtkWidget *image = gtk_image_new_from_icon_name (icon_name);

    gtk_image_set_pixel_size (GTK_IMAGE (image), 20);
    return create_icon_wrapper (image);
}

static GtkWidget *
create_icon_from_file (const gchar *path)
{
    GtkWidget *image;

    image = gtk_picture_new_for_filename (path);
    gtk_picture_set_can_shrink (GTK_PICTURE (image), TRUE);
    tbo_picture_set_contain (GTK_PICTURE (image));
    gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (image, GTK_ALIGN_CENTER);

    return create_icon_wrapper (image);
}

static GtkWidget *
create_project_icon (const gchar *relative_path)
{
    gchar *path = tbo_get_data_path (relative_path);
    GtkWidget *image = create_icon_from_file (path);
    g_free (path);
    return image;
}

static GtkWidget *
create_button (GtkWidget *image, const gchar *tooltip)
{
    GtkWidget *button = gtk_button_new ();

    gtk_button_set_child (GTK_BUTTON (button), image);
    gtk_widget_set_tooltip_text (button, tooltip);

    return button;
}

static GtkWidget *
create_toggle_button (GtkWidget *image, const gchar *tooltip)
{
    GtkWidget *button = gtk_toggle_button_new ();

    gtk_button_set_child (GTK_BUTTON (button), image);
    gtk_widget_set_tooltip_text (button, tooltip);

    return button;
}

static GtkWidget *
create_section_box (void)
{
    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_add_css_class (box, "linked");
    gtk_widget_add_css_class (box, "tbo-toolbar-section");

    return box;
}

static void
register_tool_button (TboToolbar *self, enum Tool tool, GtkWidget *button)
{
    self->tool_buttons[tool] = GTK_TOGGLE_BUTTON (button);
    g_object_set_data (G_OBJECT (button), "tool-id", GINT_TO_POINTER (tool));
    g_signal_connect (button, "toggled", G_CALLBACK (on_tool_button_toggled), self);
}

/* callbacks */
static gboolean
add_new_page (GtkWidget *widget, TboWindow *tbo)
{
    Page *page = tbo_comic_new_page (tbo->comic);
    gint index = tbo_comic_page_nth (tbo->comic, page);

    tbo_window_add_page_widget (tbo, create_darea (tbo), page);
    tbo_comic_set_current_page (tbo->comic, page);
    tbo_undo_stack_insert (tbo->undo_stack, tbo_action_page_add_new (tbo->comic, page, index));
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_window_mark_dirty (tbo);
    tbo_window_refresh_status (tbo);
    tbo_toolbar_update (tbo->toolbar);
    return FALSE;
}

static gboolean
duplicate_current_page (GtkWidget *widget, TboWindow *tbo)
{
    (void) widget;
    tbo_window_duplicate_current_page (tbo);
    return FALSE;
}

static gboolean
del_current_page (GtkWidget *widget, TboWindow *tbo)
{
    int nth = tbo_comic_page_index (tbo->comic);
    Page *page = tbo_comic_get_current_page (tbo->comic);

    if (page == NULL)
        return FALSE;

    g_object_ref (page);
    tbo_comic_del_current_page (tbo->comic);
    tbo_undo_stack_insert (tbo->undo_stack, tbo_action_page_remove_new (tbo->comic, page, nth));
    tbo_window_remove_page_widget (tbo, nth);
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_window_mark_dirty (tbo);
    tbo_window_refresh_status (tbo);
    tbo_toolbar_update (tbo->toolbar);
    g_object_unref (page);
    return FALSE;
}

static gboolean
next_page (GtkWidget *widget, TboWindow *tbo)
{
    tbo_comic_next_page (tbo->comic);
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_toolbar_update (tbo->toolbar);
    tbo_window_refresh_status (tbo);
    tbo_drawing_adjust_scroll (TBO_DRAWING (tbo->drawing));

    return FALSE;
}

static gboolean
prev_page (GtkWidget *widget, TboWindow *tbo)
{
    tbo_comic_prev_page (tbo->comic);
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_toolbar_update (tbo->toolbar);
    tbo_window_refresh_status (tbo);
    tbo_drawing_adjust_scroll (TBO_DRAWING (tbo->drawing));

    return FALSE;
}

static gboolean
zoom_100 (GtkWidget *widget, TboWindow *tbo)
{
    tbo_drawing_zoom_100 (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
zoom_fit (GtkWidget *widget, TboWindow *tbo)
{
    tbo_drawing_zoom_fit (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
zoom_in (GtkWidget *widget, TboWindow *tbo)
{
    tbo_drawing_zoom_in (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
zoom_out (GtkWidget *widget, TboWindow *tbo)
{
    tbo_drawing_zoom_out (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
add_pix (GtkWidget *widget, TboWindow *tbo)
{
    gchar *filename = tbo_file_dialog_open_image (tbo);

    if (filename != NULL)
    {
        tbo_dnd_insert_asset (tbo, filename, 0, 0);
        g_free (filename);
    }
    return FALSE;
}

static void
on_tool_button_toggled (GtkToggleButton *button, TboToolbar *toolbar)
{
    enum Tool tool;

    if (toolbar->syncing_tool_buttons)
        return;

    tool = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "tool-id"));

    if (gtk_toggle_button_get_active (button))
        tbo_toolbar_set_selected_tool (toolbar, tool);
    else if (toolbar->selected_tool == toolbar->tools[tool])
        tbo_toolbar_set_selected_tool (toolbar, TBO_TOOLBAR_NONE);
}

static GtkWidget *
generate_toolbar (TboToolbar *self)
{
    GtkWidget *toolbar;
    GtkWidget *section;

    toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_name (toolbar, "tbo-toolbar");
    gtk_widget_set_hexpand (toolbar, TRUE);
    gtk_widget_set_halign (toolbar, GTK_ALIGN_FILL);
    gtk_widget_set_margin_start (toolbar, 12);
    gtk_widget_set_margin_end (toolbar, 12);
    gtk_widget_set_margin_top (toolbar, 8);
    gtk_widget_set_margin_bottom (toolbar, 8);

    section = create_section_box ();
    self->button_new = create_button (create_project_icon ("icons/new.svg"), _("New Comic (Ctrl+N)"));
    self->button_open = create_button (create_icon_from_name ("document-open-symbolic"), _("Open Comic (Ctrl+O)"));
    self->button_save = create_button (create_icon_from_name ("document-save-symbolic"), _("Save Comic (Ctrl+S)"));
    gtk_widget_add_css_class (self->button_save, "suggested-action");
    g_signal_connect (self->button_new, "clicked", G_CALLBACK (tbo_comic_new_dialog), self->tbo);
    g_signal_connect (self->button_open, "clicked", G_CALLBACK (tbo_comic_open_dialog), self->tbo);
    g_signal_connect (self->button_save, "clicked", G_CALLBACK (tbo_comic_save_dialog), self->tbo);
    tbo_box_pack_start (section, self->button_new, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_open, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_save, FALSE, FALSE, 0);
    tbo_box_pack_start (toolbar, section, FALSE, FALSE, 0);

    section = create_section_box ();
    self->button_undo = create_button (create_project_icon ("icons/undo.svg"), _("Undo (Ctrl+Z)"));
    self->button_redo = create_button (create_project_icon ("icons/redo.svg"), _("Redo (Ctrl+Y)"));
    g_signal_connect (self->button_undo, "clicked", G_CALLBACK (tbo_window_undo_cb), self->tbo);
    g_signal_connect (self->button_redo, "clicked", G_CALLBACK (tbo_window_redo_cb), self->tbo);
    tbo_box_pack_start (section, self->button_undo, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_redo, FALSE, FALSE, 0);
    tbo_box_pack_start (toolbar, section, FALSE, FALSE, 0);

    section = create_section_box ();
    self->button_new_page = create_button (create_icon_from_name ("list-add-symbolic"), _("New Page"));
    self->button_duplicate_page = create_button (create_icon_from_name ("edit-copy-symbolic"), _("Duplicate Page"));
    self->button_delete_page = create_button (create_icon_from_name ("edit-delete-symbolic"), _("Delete Page"));
    self->button_prev_page = create_button (create_icon_from_name ("go-previous-symbolic"), _("Previous Page"));
    self->button_next_page = create_button (create_icon_from_name ("go-next-symbolic"), _("Next Page"));
    g_signal_connect (self->button_new_page, "clicked", G_CALLBACK (add_new_page), self->tbo);
    g_signal_connect (self->button_duplicate_page, "clicked", G_CALLBACK (duplicate_current_page), self->tbo);
    g_signal_connect (self->button_delete_page, "clicked", G_CALLBACK (del_current_page), self->tbo);
    g_signal_connect (self->button_prev_page, "clicked", G_CALLBACK (prev_page), self->tbo);
    g_signal_connect (self->button_next_page, "clicked", G_CALLBACK (next_page), self->tbo);
    tbo_box_pack_start (section, self->button_new_page, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_duplicate_page, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_delete_page, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_prev_page, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_next_page, FALSE, FALSE, 0);
    tbo_box_pack_start (toolbar, section, FALSE, FALSE, 0);

    section = create_section_box ();
    register_tool_button (self, TBO_TOOLBAR_SELECTOR,
                          create_toggle_button (create_project_icon ("icons/selector.svg"), _("Selector (S)")));
    register_tool_button (self, TBO_TOOLBAR_FRAME,
                          create_toggle_button (create_project_icon ("icons/frame.svg"), _("New Frame (F)")));
    register_tool_button (self, TBO_TOOLBAR_DOODLE,
                          create_toggle_button (create_project_icon ("icons/doodle.svg"), _("Doodle (D)")));
    register_tool_button (self, TBO_TOOLBAR_BUBBLE,
                          create_toggle_button (create_project_icon ("icons/bubble.svg"), _("Bubble (B)")));
    register_tool_button (self, TBO_TOOLBAR_TEXT,
                          create_toggle_button (create_project_icon ("icons/text.svg"), _("Text (T)")));
    tbo_box_pack_start (section, GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_SELECTOR]), FALSE, FALSE, 0);
    tbo_box_pack_start (section, GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_FRAME]), FALSE, FALSE, 0);
    tbo_box_pack_start (section, GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_DOODLE]), FALSE, FALSE, 0);
    tbo_box_pack_start (section, GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_BUBBLE]), FALSE, FALSE, 0);
    tbo_box_pack_start (section, GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_TEXT]), FALSE, FALSE, 0);
    tbo_box_pack_start (toolbar, section, FALSE, FALSE, 0);

    section = create_section_box ();
    self->button_pix = create_button (create_project_icon ("icons/pix.svg"), _("Insert Image"));
    g_signal_connect (self->button_pix, "clicked", G_CALLBACK (add_pix), self->tbo);
    tbo_box_pack_start (section, self->button_pix, FALSE, FALSE, 0);
    tbo_box_pack_start (toolbar, section, FALSE, FALSE, 0);

    section = create_section_box ();
    self->button_zoom_100 = create_button (create_icon_from_name ("zoom-original-symbolic"), _("Zoom 1:1 (1)"));
    self->button_zoom_out = create_button (create_icon_from_name ("zoom-out-symbolic"), _("Zoom Out (-)"));
    self->button_zoom_in = create_button (create_icon_from_name ("zoom-in-symbolic"), _("Zoom In (+)"));
    self->button_zoom_fit = create_button (create_project_icon ("icons/zoom-fit.svg"), _("Zoom Fit (2)"));
    g_signal_connect (self->button_zoom_100, "clicked", G_CALLBACK (zoom_100), self->tbo);
    g_signal_connect (self->button_zoom_out, "clicked", G_CALLBACK (zoom_out), self->tbo);
    g_signal_connect (self->button_zoom_in, "clicked", G_CALLBACK (zoom_in), self->tbo);
    g_signal_connect (self->button_zoom_fit, "clicked", G_CALLBACK (zoom_fit), self->tbo);
    tbo_box_pack_start (section, self->button_zoom_100, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_zoom_out, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_zoom_in, FALSE, FALSE, 0);
    tbo_box_pack_start (section, self->button_zoom_fit, FALSE, FALSE, 0);
    tbo_box_pack_start (toolbar, section, FALSE, FALSE, 0);

    return toolbar;
}

/* init methods */

static void
tbo_toolbar_init (TboToolbar *self)
{
    int i;

    self->tbo = NULL;
    self->selected_tool = NULL;
    self->toolbar = NULL;
    self->button_new = NULL;
    self->button_open = NULL;
    self->button_save = NULL;
    self->button_undo = NULL;
    self->button_redo = NULL;
    self->button_new_page = NULL;
    self->button_duplicate_page = NULL;
    self->button_delete_page = NULL;
    self->button_prev_page = NULL;
    self->button_next_page = NULL;
    self->button_zoom_in = NULL;
    self->button_zoom_100 = NULL;
    self->button_zoom_fit = NULL;
    self->button_zoom_out = NULL;
    self->button_pix = NULL;
    self->syncing_tool_buttons = FALSE;
    self->tools = NULL;

    for (i = 0; i < TBO_TOOLBAR_N_TOOLS; i++)
        self->tool_buttons[i] = NULL;
}

static void
tbo_toolbar_finalize (GObject *self)
{
    if (TBO_TOOLBAR (self)->tools)
    {
        int i;
        for (i=1; i < TBO_TOOLBAR_N_TOOLS; i++)
        {
            g_object_unref (G_OBJECT (TBO_TOOLBAR (self)->tools[i]));
        }
        g_free (TBO_TOOLBAR (self)->tools);
    }

    G_OBJECT_CLASS (tbo_toolbar_parent_class)->finalize (self);
}

static void
tbo_toolbar_class_init (TboToolbarClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = tbo_toolbar_finalize;
}

/* object functions */

GObject *
tbo_toolbar_new (void)
{
    GObject *toolbar;
    toolbar = g_object_new (TBO_TYPE_TOOLBAR, NULL);
    return toolbar;
}

GObject *
tbo_toolbar_new_with_params (TboWindow *tbo)
{
    GObject *obj;
    TboToolbar *toolbar;
    TboToolBase *tool;
    obj = tbo_toolbar_new ();

    toolbar = TBO_TOOLBAR (obj);
    toolbar->tbo = tbo;

    toolbar->tools = g_new (TboToolBase *, TBO_TOOLBAR_N_TOOLS);
    toolbar->tools[TBO_TOOLBAR_NONE] = NULL;

    tool = TBO_TOOL_BASE (tbo_tool_selector_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "Selector");
    toolbar->tools[TBO_TOOLBAR_SELECTOR] = tool;

    tool = TBO_TOOL_BASE (tbo_tool_frame_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "NewFrame");
    toolbar->tools[TBO_TOOLBAR_FRAME] = tool;

    tool = TBO_TOOL_BASE (tbo_tool_doodle_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "Doodle");
    toolbar->tools[TBO_TOOLBAR_DOODLE] = tool;

    tool = TBO_TOOL_BASE (tbo_tool_bubble_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "Bubble");
    toolbar->tools[TBO_TOOLBAR_BUBBLE] = tool;

    tool = TBO_TOOL_BASE (tbo_tool_text_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "Text");
    toolbar->tools[TBO_TOOLBAR_TEXT] = tool;

    toolbar->toolbar = generate_toolbar (toolbar);

    return obj;
}

TboToolBase *
tbo_toolbar_get_selected_tool (TboToolbar *self)
{
    return self->selected_tool;
}

void
tbo_toolbar_set_selected_tool (TboToolbar *self, enum Tool tool)
{
    TboToolBase *t = NULL;

    if (self->selected_tool == self->tools[tool])
        return;

    if (self->selected_tool != NULL)
        self->selected_tool->on_unselect (self->selected_tool);

    self->selected_tool = NULL;

    if (tool != TBO_TOOLBAR_NONE && self->tools[tool] != NULL)
    {
        t = self->tools[tool];
        if (gtk_widget_is_sensitive (GTK_WIDGET (self->tool_buttons[tool])))
        {
            self->selected_tool = t;
            self->selected_tool->on_select (self->selected_tool);
        }
    }

    self->syncing_tool_buttons = TRUE;
    for (int i = 1; i < TBO_TOOLBAR_N_TOOLS; i++)
    {
        gtk_toggle_button_set_active (self->tool_buttons[i],
                                      self->selected_tool == self->tools[i]);
    }
    self->syncing_tool_buttons = FALSE;

    TBO_DRAWING (self->tbo->drawing)->tool = self->selected_tool;
    tbo_toolbar_update (self);
}

GtkWidget *
tbo_toolbar_get_toolbar (TboToolbar *self)
{
    return self->toolbar;
}

void
tbo_toolbar_update (TboToolbar *self)
{
    TboWindow *tbo;
    gboolean in_frame_view;

    if (!self || self->tbo == NULL || self->tbo->destroying)
        return;

    tbo = self->tbo;
    in_frame_view = tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) != NULL;

    gtk_widget_set_sensitive (self->button_undo, tbo_undo_active_undo (tbo->undo_stack));
    gtk_widget_set_sensitive (self->button_redo, tbo_undo_active_redo (tbo->undo_stack));

    gtk_widget_set_sensitive (self->button_prev_page, !tbo_comic_page_first (tbo->comic));
    gtk_widget_set_sensitive (self->button_next_page, !tbo_comic_page_last (tbo->comic));
    gtk_widget_set_sensitive (self->button_duplicate_page, tbo_comic_len (tbo->comic) > 0);
    gtk_widget_set_sensitive (self->button_delete_page, tbo_comic_len (tbo->comic) > 1);

    gtk_widget_set_sensitive (GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_DOODLE]), in_frame_view);
    gtk_widget_set_sensitive (GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_BUBBLE]), in_frame_view);
    gtk_widget_set_sensitive (GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_TEXT]), in_frame_view);
    gtk_widget_set_sensitive (self->button_pix, in_frame_view);
    gtk_widget_set_sensitive (GTK_WIDGET (self->tool_buttons[TBO_TOOLBAR_FRAME]), !in_frame_view);

    if (!in_frame_view && self->selected_tool != NULL &&
        (self->selected_tool == self->tools[TBO_TOOLBAR_DOODLE] ||
         self->selected_tool == self->tools[TBO_TOOLBAR_BUBBLE] ||
         self->selected_tool == self->tools[TBO_TOOLBAR_TEXT]))
    {
        tbo_toolbar_set_selected_tool (self, TBO_TOOLBAR_SELECTOR);
        return;
    }

    update_menubar (tbo);
}
