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


#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "config.h"
#include "ui-menu.h"
#include "comic-new-dialog.h"
#include "comic-saveas-dialog.h"
#include "comic-open-dialog.h"
#include "tbo-window.h"
#include "tbo-drawing.h"
#include "export.h"
#include "tbo-tool-selector.h"
#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-base.h"

static GtkActionGroup *MENU_ACTION_GROUP = NULL;

void
update_menubar (TboWindow *tbo)
{
    GtkAction *action;
    int i;
    char *actions[20] = {"FlipHObj", "FlipVObj", "OrderUpObj", "OrderDownObj", "DeleteObj", "CloneObj"};
    int elements = 6;
    int obj_n_elements = 4;
    gboolean activated = FALSE;

    if (!tbo->toolbar)
        return;

    TboDrawing *drawing = TBO_DRAWING (tbo->drawing);
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *frame = selector->selected_frame;

    if (!MENU_ACTION_GROUP)
        return;

    if (tbo_drawing_get_current_frame (drawing) && obj)
    {
        for (i=0; i<elements; i++)
        {
            action = gtk_action_group_get_action (MENU_ACTION_GROUP, actions[i]);
            gtk_action_set_sensitive (action, TRUE);
        }
    }
    else if (!tbo_drawing_get_current_frame (drawing) && frame)
    {
        for (i=obj_n_elements; i<elements; i++)
        {
            action = gtk_action_group_get_action (MENU_ACTION_GROUP, actions[i]);
            gtk_action_set_sensitive (action, TRUE);
        }
    }
    else
    {
        for (i=0; i<elements; i++)
        {
            action = gtk_action_group_get_action (MENU_ACTION_GROUP, actions[i]);
            gtk_action_set_sensitive (action, FALSE);
        }
    }
}

gboolean
clone_obj_cb (GtkWidget *widget, TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *frame = selector->selected_frame;
    Page *page = tbo_comic_get_current_page (tbo->comic);
    TboDrawing *drawing = TBO_DRAWING (tbo->drawing);

    if (!tbo_drawing_get_current_frame (drawing) && frame)
    {
        Frame *cloned_frame = tbo_frame_clone (frame);
        cloned_frame->x += 10;
        cloned_frame->y -= 10;
        tbo_page_add_frame (page, cloned_frame);
        tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
        tbo_tool_selector_set_selected (selector, cloned_frame);
    }
    else if (obj && tbo_drawing_get_current_frame (drawing))
    {
        TboObjectBase *cloned_obj = obj->clone (obj);
        cloned_obj->x += 10;
        cloned_obj->y -= 10;
        tbo_frame_add_obj (frame, cloned_obj);
        tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
        tbo_tool_selector_set_selected_obj (selector, cloned_obj);
    }
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

gboolean
delete_obj_cb (GtkWidget *widget, TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *frame = selector->selected_frame;
    Page *page = tbo_comic_get_current_page (tbo->comic);
    TboDrawing *drawing = TBO_DRAWING (tbo->drawing);

    if (obj && tbo_drawing_get_current_frame (drawing))
    {
        tbo_frame_del_obj (frame, obj);
        tbo_tool_selector_set_selected_obj (selector, NULL);
    }
    else if (!tbo_drawing_get_current_frame (drawing) && frame)
    {
        tbo_page_del_frame (page, frame);
        tbo_tool_selector_set_selected (selector, NULL);
    }
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

gboolean
flip_v_cb (GtkWidget *widget, TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    if (obj)
        tbo_object_base_flipv (obj);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

gboolean
flip_h_cb (GtkWidget *widget, TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    if (obj)
        tbo_object_base_fliph (obj);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

gboolean
order_up_cb (GtkWidget *widget, TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *current_frame = selector->selected_frame;
    if (obj)
        tbo_object_base_order_up (obj, current_frame);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

gboolean
order_down_cb (GtkWidget *widget, TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *current_frame = selector->selected_frame;
    if (obj)
        tbo_object_base_order_down (obj, current_frame);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

gboolean close_cb (GtkWidget *widget, TboWindow *tbo)
{
    tbo_window_free_cb (widget, NULL, tbo);
    return FALSE;
}

gboolean
tutorial_cb (GtkWidget *widget, TboWindow *tbo){
    char *filename = DATA_DIR "/tut.tbo";
    tbo_comic_open (tbo, filename);
    tbo_window_set_path (tbo, filename);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    tbo_window_update_status (tbo, 0, 0);
    return FALSE;
}

gboolean
about_cb (GtkWidget *widget, TboWindow *tbo){
    const gchar *authors[] = {"danigm <dani@danigm.net>", NULL};
    const gchar *artists[] = {"danigm <dani@danigm.net>",
                              "",
                              "Arcadia http://www.arcadiaproject.org :",
                              "Samuel Navas Portillo",
                              "Daniel Pavón Pérez",
                              "Juan Jesús Pérez Luna",
                              "",
                              "Zapatero & Rajoy:",
                              "Alfonso de Cala",
                              "",
                              "South park style:",
                              "Rafael García <bladecoder@gmail.com>",
                              "",
                              "Facilware:",
                              "VIcente Pons <simpons@gmail.com>",
                              NULL};

    gtk_show_about_dialog (GTK_WINDOW (tbo->window),
            "name", _("TBO comic editor"),
            "version", VERSION,
            "authors", authors,
            "artists", artists,
            "website", "http://trac.danigm.net/tbo",
            "translator-credits", _("translator-credits"),
            NULL);

    return FALSE;
}

gboolean
tbo_menu_export (GtkWidget *widget, TboWindow *tbo)
{
    tbo_export (tbo);
    return FALSE;
}

static const GtkActionEntry tbo_menu_entries [] = {
    /* Toplevel */

    { "File", NULL, N_("_File") },
    { "Edit", NULL, N_("_Edit") },
    { "Help", NULL, N_("Help") },

    /* File menu */

    { "NewFile", GTK_STOCK_NEW, N_("_New"), "<control>N",
      N_("Create a new file"),
      G_CALLBACK (tbo_comic_new_dialog) },

    { "OpenFile", GTK_STOCK_OPEN, N_("_Open"), "<control>O",
      N_("Open a new file"),
      G_CALLBACK (tbo_comic_open_dialog) },

    { "SaveFile", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
      N_("Save current document"),
      G_CALLBACK (tbo_comic_save_dialog) },

    { "SaveFileAs", GTK_STOCK_SAVE_AS, N_("_Save as"), "",
      N_("Save current document as ..."),
      G_CALLBACK (tbo_comic_saveas_dialog) },

    { "ToPNG", GTK_STOCK_FILE, N_("Export as..."), "",
      N_("Save current document as..."),
      G_CALLBACK (tbo_menu_export) },

    { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
      N_("Quit"),
      G_CALLBACK (close_cb) },

    /* edit menu */

    { "CloneObj", GTK_STOCK_COPY, N_("Clone"), "<control>d",
      N_("Clone"),
      G_CALLBACK (clone_obj_cb) },
    { "DeleteObj", GTK_STOCK_DELETE, N_("Delete"), "Delete",
      N_("Delete"),
      G_CALLBACK (delete_obj_cb) },
    { "FlipHObj", NULL, N_("Horizontal flip"), "h",
      N_("Horizontal flip"),
      G_CALLBACK (flip_h_cb) },
    { "FlipVObj", NULL, N_("Vertical flip"), "v",
      N_("Vertical flip"),
      G_CALLBACK (flip_v_cb) },
    { "OrderUpObj", NULL, N_("Move to front"), "Page_Up",
      N_("Move to front"),
      G_CALLBACK ( order_up_cb ) },
    { "OrderDownObj", NULL, N_("Move to back"), "Page_Down",
      N_("Move to back"),
      G_CALLBACK ( order_down_cb ) },

    /* Help menu */
    { "Tutorial", NULL, N_("Tutorial"), "",
      N_("Tutorial"),
      G_CALLBACK (tutorial_cb) },

    { "About", GTK_STOCK_ABOUT, N_("About"), "",
      N_("About"),
      G_CALLBACK (about_cb) },
};

GtkWidget *generate_menu (TboWindow *window){
    GtkWidget *menu;
    GtkUIManager *manager;
    GtkAccelGroup *accel;
    GError *error = NULL;

    manager = gtk_ui_manager_new ();
    gtk_ui_manager_add_ui_from_file (manager, DATA_DIR "/ui/tbo-menu-ui.xml", &error);
    if (error != NULL)
    {
        g_warning (_("Could not merge tbo-menu-ui.xml: %s"), error->message);
        g_error_free (error);
    }

    MENU_ACTION_GROUP = gtk_action_group_new ("MenuActions");
    gtk_action_group_set_translation_domain (MENU_ACTION_GROUP, NULL);
    gtk_action_group_add_actions (MENU_ACTION_GROUP, tbo_menu_entries,
                        G_N_ELEMENTS (tbo_menu_entries), window);

    gtk_ui_manager_insert_action_group (manager, MENU_ACTION_GROUP, 0);

    menu = gtk_ui_manager_get_widget (manager, "/menubar");

    accel = gtk_ui_manager_get_accel_group (manager);
    gtk_window_add_accel_group (GTK_WINDOW (window->window), accel);

    return menu;
}

