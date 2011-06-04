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
#include "tbo-window.h"
#include "tbo-toolbar.h"
#include "comic.h"
#include "tbo-object-pixmap.h"
#include "comic-new-dialog.h"
#include "comic-open-dialog.h"
#include "comic-saveas-dialog.h"
#include "custom-stock.h"
#include "tbo-drawing.h"
#include "frame.h"
#include "tbo-tool-selector.h"
#include "tbo-tool-frame.h"
#include "tbo-tool-doodle.h"
#include "tbo-tool-bubble.h"
#include "tbo-tool-text.h"
#include "ui-menu.h"
#include "tbo-undo.h"

G_DEFINE_TYPE (TboToolbar, tbo_toolbar, G_TYPE_OBJECT);


static gboolean select_tool (GtkAction *action, TboToolbar *toolbar);

/* callbacks */
static gboolean
add_new_page (GtkAction *action, TboWindow *tbo)
{
    Page *page = tbo_comic_new_page (tbo->comic);
    int nth = tbo_comic_page_nth (tbo->comic, page);
    gtk_notebook_insert_page (GTK_NOTEBOOK (tbo->notebook),
                              create_darea (tbo),
                              NULL,
                              nth);
    tbo_window_update_status (tbo, 0, 0);
    tbo_toolbar_update (tbo->toolbar);
    return FALSE;
}

static gboolean
del_current_page (GtkAction *action, TboWindow *tbo)
{
    int nth = tbo_comic_page_index (tbo->comic);
    tbo_comic_del_current_page (tbo->comic);
    tbo_window_set_current_tab_page (tbo, TRUE);
    gtk_notebook_remove_page (GTK_NOTEBOOK (tbo->notebook), nth);
    tbo_window_update_status (tbo, 0, 0);
    tbo_toolbar_update (tbo->toolbar);
    return FALSE;
}

static gboolean
next_page (GtkAction *action, TboWindow *tbo)
{
    tbo_comic_next_page (tbo->comic);
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_toolbar_update (tbo->toolbar);
    tbo_window_update_status (tbo, 0, 0);
    tbo_drawing_adjust_scroll (TBO_DRAWING (tbo->drawing));

    return FALSE;
}

static gboolean
prev_page (GtkAction *action, TboWindow *tbo)
{
    tbo_comic_prev_page (tbo->comic);
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_toolbar_update (tbo->toolbar);
    tbo_window_update_status (tbo, 0, 0);
    tbo_drawing_adjust_scroll (TBO_DRAWING (tbo->drawing));

    return FALSE;
}

static gboolean
zoom_100 (GtkAction *action, TboWindow *tbo)
{
    tbo_drawing_zoom_100 (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
zoom_fit (GtkAction *action, TboWindow *tbo)
{
    tbo_drawing_zoom_fit (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
zoom_in (GtkAction *action, TboWindow *tbo)
{
    tbo_drawing_zoom_in (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
zoom_out (GtkAction *action, TboWindow *tbo)
{
    tbo_drawing_zoom_out (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
add_pix (GtkAction *action, TboWindow *tbo)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;

    dialog = gtk_file_chooser_dialog_new (_("Add an Image"),
                     GTK_WINDOW (tbo->window),
                     GTK_FILE_CHOOSER_ACTION_OPEN,
                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                     NULL);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("png"));
    gtk_file_filter_add_pattern (filter, "*.png");
    gtk_file_filter_add_pattern (filter, "*.PNG");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        TboObjectPixmap *piximage = TBO_OBJECT_PIXMAP (tbo_object_pixmap_new_with_params (0, 0, 0, 0, filename));
        tbo_frame_add_obj (tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)), TBO_OBJECT_BASE (piximage));
        tbo_drawing_update (TBO_DRAWING (tbo->drawing));
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
    return FALSE;
}

/* actions */

static const GtkActionEntry tbo_tools_entries [] = {
    { "NewFileTool", GTK_STOCK_NEW, N_("_New"), "<control>N",
      N_("New Comic"),
      G_CALLBACK (tbo_comic_new_dialog) },

    { "OpenFileTool", GTK_STOCK_OPEN, N_("_Open"), "<control>O",
      N_("Open comic"),
      G_CALLBACK (tbo_comic_open_dialog) },

    { "SaveFileTool", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
      N_("Save current document"),
      G_CALLBACK (tbo_comic_save_dialog) },

    // Undo and Redo
    { "Undo", GTK_STOCK_UNDO, N_("_Undo"), "<control>Z",
      N_("Undo the last action"),
      G_CALLBACK (tbo_window_undo_cb) },
    { "Redo", GTK_STOCK_REDO, N_("_Redo"), "<control>Y",
      N_("Undo the last action"),
      G_CALLBACK (tbo_window_redo_cb) },

    // Page tools
    { "NewPage", GTK_STOCK_ADD, N_("New Page"), "<control>P",
      N_("New page"),
      G_CALLBACK (add_new_page) },

    { "DelPage", GTK_STOCK_DELETE, N_("Delete Page"), "",
      N_("Delete current page"),
      G_CALLBACK (del_current_page) },

    { "PrevPage", GTK_STOCK_GO_BACK, N_("Prev Page"), "",
      N_("Prev page"),
      G_CALLBACK (prev_page) },

    { "NextPage", GTK_STOCK_GO_FORWARD, N_("Next Page"), "",
      N_("Next page"),
      G_CALLBACK (next_page) },

    // Zoom tools
    { "Zoomin", GTK_STOCK_ZOOM_IN, N_("Zoom in"), "",
      N_("Zoom in"),
      G_CALLBACK (zoom_in) },
    { "Zoom100", GTK_STOCK_ZOOM_100, N_("Zoom 1:1"), "",
      N_("Zoom 1:1"),
      G_CALLBACK (zoom_100) },
    { "Zoomfit", GTK_STOCK_ZOOM_FIT, N_("Zoom fit"), "",
      N_("Zoom fit"),
      G_CALLBACK (zoom_fit) },
    { "Zoomout", GTK_STOCK_ZOOM_OUT, N_("Zoom out"), "",
      N_("Zoom out"),
      G_CALLBACK (zoom_out) },

    // Png image tool
    { "Pix", TBO_STOCK_PIX, N_("Image"), "",
      N_("Image"),
      G_CALLBACK (add_pix) },
};

/* toggle actions */
static const GtkToggleActionEntry tbo_tools_toggle_entries [] = {
    // Page view tools
    { "NewFrame", TBO_STOCK_FRAME, N_("New _Frame"), "f",
      N_("New Frame"),
      G_CALLBACK (select_tool), FALSE },

    { "Selector", TBO_STOCK_SELECTOR, N_("Selector"), "s",
      N_("Selector"),
      G_CALLBACK (select_tool), FALSE },

    // Frame view tools
    { "Doodle", TBO_STOCK_DOODLE, N_("Doodle"), "d",
      N_("Doodle"),
      G_CALLBACK (select_tool), FALSE },
    { "Bubble", TBO_STOCK_BUBBLE, N_("Booble"), "b",
      N_("Bubble"),
      G_CALLBACK (select_tool), FALSE },
    { "Text", TBO_STOCK_TEXT, N_("Text"), "t",
      N_("Text"),
      G_CALLBACK (select_tool), FALSE },
};

/* aux */

static void
unselect_tool (TboToolbar *self)
{
    GtkToggleAction *action;

    if (!self->selected_tool)
        return;

    self->selected_tool->on_unselect (self->selected_tool);
    action = (GtkToggleAction *) gtk_action_group_get_action (self->action_group,
                                                    self->selected_tool->action);
    gtk_toggle_action_set_active (action, FALSE);
}

static gboolean
select_tool (GtkAction *action, TboToolbar *toolbar)
{
    GtkToggleAction *toggle_action;
    int i;
    const gchar *name;
    TboWindow *tbo = toolbar->tbo;
    TboToolBase *tool;

    toggle_action = (GtkToggleAction *) action;
    name = gtk_action_get_name (action);

    /* starting at 1 because 0 is NULL, TBO_TOOLBAR_NONE */
    for (i=1; i < TBO_TOOLBAR_N_TOOLS; i++)
    {
        tool = toolbar->tools[i];
        if (strcmp (tool->action, name) == 0)
            break;
    }

    if (gtk_toggle_action_get_active (toggle_action))
        tbo_toolbar_set_selected_tool (toolbar, i);
    else
        tbo_toolbar_set_selected_tool (toolbar, TBO_TOOLBAR_NONE);
    tbo_window_update_status (tbo, 0, 0);
    return FALSE;
}

static GtkWidget *
generate_toolbar (TboToolbar *self)
{
    GtkWidget *toolbar;
    GtkUIManager *manager;
    GError *error = NULL;

    manager = gtk_ui_manager_new ();
    gtk_ui_manager_add_ui_from_file (manager, DATA_DIR "/ui/tbo-toolbar-ui.xml", &error);
    if (error != NULL)
    {
        g_warning ("Could not merge tbo-toolbar-ui.xml: %s", error->message);
        g_error_free (error);
    }

    self->action_group = gtk_action_group_new ("ToolsActions");
    gtk_action_group_set_translation_domain (self->action_group, NULL);
    gtk_action_group_add_actions (self->action_group, tbo_tools_entries,
                        G_N_ELEMENTS (tbo_tools_entries), self->tbo);
    gtk_action_group_add_toggle_actions (self->action_group, tbo_tools_toggle_entries,
                        G_N_ELEMENTS (tbo_tools_toggle_entries), self);

    gtk_ui_manager_insert_action_group (manager, self->action_group, 0);

    toolbar = gtk_ui_manager_get_widget (manager, "/toolbar");
    return toolbar;
}


/* init methods */

static void
tbo_toolbar_init (TboToolbar *self)
{
    self->tbo = NULL;
    self->selected_tool = NULL;
    self->action_group = NULL;
    self->tools = NULL;
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

    if (TBO_TOOLBAR (self)->toolbar)
        g_object_unref (G_OBJECT (TBO_TOOLBAR (self)->toolbar));
    /* Chain up to the parent class */
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
tbo_toolbar_new ()
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
    /* Adding tools */

    toolbar->tools = g_new (TboToolBase*, TBO_TOOLBAR_N_TOOLS);
    toolbar->tools[TBO_TOOLBAR_NONE] = NULL;

    /* selector */
    tool = TBO_TOOL_BASE (tbo_tool_selector_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "Selector");
    toolbar->tools[TBO_TOOLBAR_SELECTOR] = tool;

    /* frame */
    tool = TBO_TOOL_BASE (tbo_tool_frame_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "NewFrame");
    toolbar->tools[TBO_TOOLBAR_FRAME] = tool;

    /* doodle */
    tool = TBO_TOOL_BASE (tbo_tool_doodle_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "Doodle");
    toolbar->tools[TBO_TOOLBAR_DOODLE] = tool;

    /* bubble */
    tool = TBO_TOOL_BASE (tbo_tool_bubble_new_with_params (tbo));
    tbo_tool_base_set_action (tool, "Bubble");
    toolbar->tools[TBO_TOOLBAR_BUBBLE] = tool;

    /* text */
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
    GtkToggleAction *action;
    TboToolBase *t;

    if (self->selected_tool == self->tools[tool])
        return;

    unselect_tool (self);
    self->selected_tool = NULL;
    if (self->tools[tool])
    {
        t = self->tools[tool];
        action = GTK_TOGGLE_ACTION (gtk_action_group_get_action (self->action_group, t->action));

        if (gtk_action_is_sensitive (GTK_ACTION (action)))
        {
            self->selected_tool = t;
            self->selected_tool->on_select (self->selected_tool);
            gtk_toggle_action_set_active (action, TRUE);
        }
    }
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
    GtkAction *prev;
    GtkAction *next;
    GtkAction *delete;

    GtkAction *doodle;
    GtkAction *bubble;
    GtkAction *text;
    GtkAction *new_frame;
    GtkAction *pix;

    GtkAction *undo;
    GtkAction *redo;

    if (!self)
        return;

    TboWindow *tbo = self->tbo;

    if (!self->action_group)
        return;

    undo = gtk_action_group_get_action (self->action_group, "Undo");
    redo = gtk_action_group_get_action (self->action_group, "Redo");

    gtk_action_set_sensitive (undo, tbo_undo_active_undo (tbo->undo_stack));
    gtk_action_set_sensitive (redo, tbo_undo_active_redo (tbo->undo_stack));

    // Page next, prev and delete button sensitive
    prev = gtk_action_group_get_action (self->action_group, "PrevPage");
    next = gtk_action_group_get_action (self->action_group, "NextPage");
    delete = gtk_action_group_get_action (self->action_group, "DelPage");

    if (tbo_comic_page_first (tbo->comic))
        gtk_action_set_sensitive (prev, FALSE);
    else
        gtk_action_set_sensitive (prev, TRUE);

    if (tbo_comic_page_last (tbo->comic))
        gtk_action_set_sensitive (next, FALSE);
    else
        gtk_action_set_sensitive (next, TRUE);
    if (tbo_comic_len (tbo->comic) > 1)
        gtk_action_set_sensitive (delete, TRUE);
    else
        gtk_action_set_sensitive (delete, FALSE);

    // Frame view disabled in page view
    doodle = gtk_action_group_get_action (self->action_group, "Doodle");
    bubble = gtk_action_group_get_action (self->action_group, "Bubble");
    text = gtk_action_group_get_action (self->action_group, "Text");
    new_frame = gtk_action_group_get_action (self->action_group, "NewFrame");
    pix = gtk_action_group_get_action (self->action_group, "Pix");

    if (!tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)))
    {
        gtk_action_set_sensitive (doodle, FALSE);
        gtk_action_set_sensitive (bubble, FALSE);
        gtk_action_set_sensitive (text, FALSE);
        gtk_action_set_sensitive (pix, FALSE);
        gtk_action_set_sensitive (new_frame, TRUE);
    }
    else
    {
        gtk_action_set_sensitive (doodle, TRUE);
        gtk_action_set_sensitive (bubble, TRUE);
        gtk_action_set_sensitive (text, TRUE);
        gtk_action_set_sensitive (pix, TRUE);
        gtk_action_set_sensitive (new_frame, FALSE);
    }
    update_menubar (tbo);
}
