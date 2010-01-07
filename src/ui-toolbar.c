#include <stdio.h>
#include <gtk/gtk.h>
#include "ui-menu.h"
#include "ui-toolbar.h"
#include "config.h"
#include "comic-new-dialog.h"
#include "tbo-window.h"
#include "comic.h"

static int SELECTED_TOOL = NONE;
static GtkActionGroup *ACTION_GROUP;

enum Tool
get_selected_tool ()
{
    return SELECTED_TOOL;
}

void
set_selected_tool (enum Tool tool)
{
    SELECTED_TOOL = tool;
}

void
update_toolbar (TboWindow *tbo)
{
    GtkAction *prev;
    GtkAction *next;

    // Page next and prev button sensitive
    prev = gtk_action_group_get_action (ACTION_GROUP, "PrevPage");
    next = gtk_action_group_get_action (ACTION_GROUP, "NextPage");

    if (tbo_comic_page_first (tbo->comic))
        gtk_action_set_sensitive (prev, FALSE);
    else
        gtk_action_set_sensitive (prev, TRUE);

    if (tbo_comic_page_last (tbo->comic))
        gtk_action_set_sensitive (next, FALSE);
    else
        gtk_action_set_sensitive (next, TRUE);

}

gboolean 
toolbar_handler (GtkWidget *widget, gpointer data)
{
    printf("toolbar: %s\n", ((TboWindow *)data)->comic->title);
    return FALSE;
}

gboolean
add_new_page (GtkAction *action, TboWindow *tbo)
{
    tbo_comic_new_page (tbo->comic);
    tbo_window_update_status (tbo, 0, 0);
    update_toolbar (tbo);
    return FALSE;
}

gboolean 
next_page (GtkAction *action, TboWindow *tbo)
{
    tbo_comic_next_page (tbo->comic);
    update_toolbar (tbo);
    tbo_window_update_status (tbo, 0, 0);

    return FALSE;
}

gboolean
prev_page (GtkAction *action, TboWindow *tbo)
{
    tbo_comic_prev_page (tbo->comic);
    update_toolbar (tbo);
    tbo_window_update_status (tbo, 0, 0);

    return FALSE;
}

gboolean
add_new_frame (GtkAction *action, TboWindow *tbo)
{
    GtkToggleAction *frame_tool;

    frame_tool = (GtkToggleAction *)gtk_action_group_get_action (ACTION_GROUP, "NewFrame");
    if (gtk_toggle_action_get_active (frame_tool))
        set_selected_tool (FRAME);
    else
        set_selected_tool (NONE);
    update_toolbar (tbo);
    tbo_window_update_status (tbo, 0, 0);
}

static const GtkActionEntry tbo_tools_entries [] = {
    { "NewFileTool", GTK_STOCK_NEW, "_New", "<control>N",
      "New Comic",
      G_CALLBACK (tbo_comic_new_dialog) },

    { "OpenFileTool", GTK_STOCK_OPEN, "_Open", "<control>O",
      "Open comic",
      G_CALLBACK (toolbar_handler) },

    { "SaveFileTool", GTK_STOCK_SAVE, "_Save", "<control>S",
      "Save current document",
      G_CALLBACK (toolbar_handler) },

    { "NewPage", GTK_STOCK_ADD, "New Page", "<control>P",
      "New page",
      G_CALLBACK (add_new_page) },

    { "PrevPage", GTK_STOCK_GO_BACK, "Prev Page", "",
      "Prev page",
      G_CALLBACK (prev_page) },

    { "NextPage", GTK_STOCK_GO_FORWARD, "Next Page", "",
      "Next page",
      G_CALLBACK (next_page) },
};

static const GtkToggleActionEntry tbo_tools_toogle_entries [] = {
    { "NewFrame", GTK_STOCK_DND_MULTIPLE, "New _Frame", "<control>F",
      "New Frame",
      G_CALLBACK (add_new_frame), FALSE },
};

GtkWidget *generate_toolbar (TboWindow *window){
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

    ACTION_GROUP = gtk_action_group_new ("ToolsActions");
    gtk_action_group_add_actions (ACTION_GROUP, tbo_tools_entries,
                        G_N_ELEMENTS (tbo_tools_entries), window);
    gtk_action_group_add_toggle_actions (ACTION_GROUP, tbo_tools_toogle_entries,
                        G_N_ELEMENTS (tbo_tools_toogle_entries), window);

    gtk_ui_manager_insert_action_group (manager, ACTION_GROUP, 0);

    toolbar = gtk_ui_manager_get_widget (manager, "/toolbar");

    update_toolbar (window);
    
    return toolbar;
}

