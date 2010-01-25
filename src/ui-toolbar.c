#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "ui-menu.h"
#include "ui-toolbar.h"
#include "config.h"
#include "comic-new-dialog.h"
#include "tbo-window.h"
#include "comic.h"
#include "custom-stock.h"

static int SELECTED_TOOL = NONE;
static GtkActionGroup *ACTION_GROUP;

typedef struct
{
    enum Tool tool;
    char *action;

} tool_and_action;

void unselect (enum Tool tool);
gboolean select_tool (GtkAction *action, TboWindow *tbo);

enum Tool
get_selected_tool ()
{
    return SELECTED_TOOL;
}

void
set_selected_tool (enum Tool tool)
{
    unselect (SELECTED_TOOL);
    SELECTED_TOOL = tool;
}

void
update_toolbar (TboWindow *tbo)
{
    GtkAction *prev;
    GtkAction *next;
    GtkAction *delete;

    // Page next, prev and delete button sensitive
    prev = gtk_action_group_get_action (ACTION_GROUP, "PrevPage");
    next = gtk_action_group_get_action (ACTION_GROUP, "NextPage");
    delete = gtk_action_group_get_action (ACTION_GROUP, "DelPage");

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
}

gboolean 
toolbar_handler (GtkWidget *widget, gpointer data)
{
    printf(_("toolbar: %s\n"), ((TboWindow *)data)->comic->title);
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
del_current_page (GtkAction *action, TboWindow *tbo)
{
    tbo_comic_del_current_page (tbo->comic);
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


static const GtkActionEntry tbo_tools_entries [] = {
    { "NewFileTool", GTK_STOCK_NEW, N_("_New"), "<control>N",
      N_("New Comic"),
      G_CALLBACK (tbo_comic_new_dialog) },

    { "OpenFileTool", GTK_STOCK_OPEN, N_("_Open"), "<control>O",
      N_("Open comic"),
      G_CALLBACK (toolbar_handler) },

    { "SaveFileTool", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
      N_("Save current document"),
      G_CALLBACK (toolbar_handler) },

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
};

static const GtkToggleActionEntry tbo_tools_toogle_entries [] = {
    { "NewFrame", TBO_STOCK_FRAME, N_("New _Frame"), "<control>F",
      N_("New Frame"),
      G_CALLBACK (select_tool), FALSE },

    { "Selector", TBO_STOCK_SELECTOR, N_("Selector"), "",
      N_("Selector"),
      G_CALLBACK (select_tool), FALSE },
};

static const tool_and_action tools_actions [] = {
    {FRAME, "NewFrame"},
    {SELECTOR, "Selector"},
};

void
unselect (enum Tool tool)
{
    int i;
    GtkToggleAction *action;

    for (i=0; i<G_N_ELEMENTS (tools_actions); i++)
    {
        if (tools_actions[i].tool == tool)
        {
            action = (GtkToggleAction *) gtk_action_group_get_action (ACTION_GROUP, 
                    tools_actions[i].action);

            gtk_toggle_action_set_active (action, FALSE);
            break;
        }
    }
}

gboolean
select_tool (GtkAction *action, TboWindow *tbo)
{
    GtkToggleAction *toggle_action;
    int i;
    const gchar *name;
    enum Tool tool;

    toggle_action = (GtkToggleAction *) action;
    name = gtk_action_get_name (action);


    for (i=0; i<G_N_ELEMENTS (tools_actions); i++)
    {
        if (strcmp (tools_actions[i].action, name) == 0)
        {
            tool = tools_actions[i].tool;
            break;
        }
    }

    if (gtk_toggle_action_get_active (toggle_action))
        set_selected_tool (tool);
    else
        set_selected_tool (NONE);
    update_toolbar (tbo);
    tbo_window_update_status (tbo, 0, 0);
    return FALSE;
}

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
    gtk_action_group_set_translation_domain (ACTION_GROUP, NULL);
    gtk_action_group_add_actions (ACTION_GROUP, tbo_tools_entries,
                        G_N_ELEMENTS (tbo_tools_entries), window);
    gtk_action_group_add_toggle_actions (ACTION_GROUP, tbo_tools_toogle_entries,
                        G_N_ELEMENTS (tbo_tools_toogle_entries), window);

    gtk_ui_manager_insert_action_group (manager, ACTION_GROUP, 0);

    toolbar = gtk_ui_manager_get_widget (manager, "/toolbar");

    update_toolbar (window);
    
    return toolbar;
}

