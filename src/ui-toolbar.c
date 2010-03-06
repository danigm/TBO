#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "ui-menu.h"
#include "ui-toolbar.h"
#include "config.h"
#include "comic-new-dialog.h"
#include "comic-saveas-dialog.h"
#include "comic-open-dialog.h"
#include "tbo-window.h"
#include "comic.h"
#include "custom-stock.h"
#include "ui-drawing.h"

#include "frame-tool.h"
#include "selector-tool.h"
#include "doodle-tool.h"
#include "text-tool.h"

static int SELECTED_TOOL = NONE;
static GtkActionGroup *ACTION_GROUP = NULL;

static ToolStruct TOOLS[] =
{
    {FRAME,
     frame_tool_on_select,
     frame_tool_on_unselect,
     frame_tool_on_move,
     frame_tool_on_click,
     frame_tool_on_release,
     frame_tool_on_key,
     frame_tool_drawing},

    {SELECTOR,
     selector_tool_on_select,
     selector_tool_on_unselect,
     selector_tool_on_move,
     selector_tool_on_click,
     selector_tool_on_release,
     selector_tool_on_key,
     selector_tool_drawing},

    {DOODLE,
     doodle_tool_on_select,
     doodle_tool_on_unselect,
     doodle_tool_on_move,
     doodle_tool_on_click,
     doodle_tool_on_release,
     doodle_tool_on_key,
     doodle_tool_drawing},

    {TEXT,
     text_tool_on_select,
     text_tool_on_unselect,
     text_tool_on_move,
     text_tool_on_click,
     text_tool_on_release,
     text_tool_on_key,
     text_tool_drawing},
};

typedef struct
{
    enum Tool tool;
    char *action;

} tool_and_action;

void unselect (enum Tool tool, TboWindow *tbo);
gboolean select_tool (GtkAction *action, TboWindow *tbo);
void update_toolbar (TboWindow *tbo);

enum Tool
get_selected_tool ()
{
    return SELECTED_TOOL;
}

void
set_selected_tool (enum Tool tool, TboWindow *tbo)
{
    unselect (SELECTED_TOOL, tbo);
    SELECTED_TOOL = tool;

    tool_signal (tool, TOOL_SELECT, tbo);
    update_toolbar (tbo);
}

void
update_toolbar (TboWindow *tbo)
{
    GtkAction *prev;
    GtkAction *next;
    GtkAction *delete;

    GtkAction *doodle;
    GtkAction *text;
    GtkAction *new_frame;

    if (!ACTION_GROUP)
        return;

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

    // Frame view disabled in page view
    doodle = gtk_action_group_get_action (ACTION_GROUP, "Doodle");
    text = gtk_action_group_get_action (ACTION_GROUP, "Text");
    new_frame = gtk_action_group_get_action (ACTION_GROUP, "NewFrame");

    if (get_frame_view() == NULL)
    {
        gtk_action_set_sensitive (doodle, FALSE);
        gtk_action_set_sensitive (text, FALSE);
        gtk_action_set_sensitive (new_frame, TRUE);
    }
    else
    {
        gtk_action_set_sensitive (doodle, TRUE);
        gtk_action_set_sensitive (text, TRUE);
        gtk_action_set_sensitive (new_frame, FALSE);
    }
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
      G_CALLBACK (tbo_comic_open_dialog) },

    { "SaveFileTool", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
      N_("Save current document"),
      G_CALLBACK (tbo_comic_save_dialog) },

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
    // Page view tools
    { "NewFrame", TBO_STOCK_FRAME, N_("New _Frame"), "<control>F",
      N_("New Frame"),
      G_CALLBACK (select_tool), FALSE },

    { "Selector", TBO_STOCK_SELECTOR, N_("Selector"), "",
      N_("Selector"),
      G_CALLBACK (select_tool), FALSE },

    // Frame view tools
    { "Doodle", TBO_STOCK_DOODLE, N_("Doodle"), "",
      N_("Doodle"),
      G_CALLBACK (select_tool), FALSE },
    { "Text", TBO_STOCK_TEXT, N_("Text"), "",
      N_("Text"),
      G_CALLBACK (select_tool), FALSE },
};

static const tool_and_action tools_actions [] = {
    {FRAME, "NewFrame"},
    {SELECTOR, "Selector"},
    {DOODLE, "Doodle"},
    {TEXT, "Text"},
};

void
unselect (enum Tool tool, TboWindow *tbo)
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
    tool_signal (tool, TOOL_UNSELECT, tbo);
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
        set_selected_tool (tool, tbo);
    else
        set_selected_tool (NONE, tbo);
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

void
tool_signal (enum Tool tool, enum ToolSignal signal, gpointer data)
{
    int i;
    ToolStruct *toolstruct = NULL;
    void **pdata;

    for (i=0; i<G_N_ELEMENTS (TOOLS); i++)
    {
        if (tool == TOOLS[i].tool)
        {
            toolstruct = &TOOLS[i];
            break;
        }
    }

    if (toolstruct)
    {
        switch (signal)
        {
            case TOOL_SELECT:
                toolstruct->tool_on_select(data);
                break;
            case TOOL_UNSELECT:
                toolstruct->tool_on_unselect(data);
                break;
            case TOOL_MOVE:
                pdata = data;
                toolstruct->tool_on_move (pdata[0], pdata[1], pdata[2]);
                break;
            case TOOL_CLICK:
                pdata = data;
                toolstruct->tool_on_click (pdata[0], pdata[1], pdata[2]);
                break;
            case TOOL_RELEASE:
                pdata = data;
                toolstruct->tool_on_release (pdata[0], pdata[1], pdata[2]);
                break;
            case TOOL_KEY:
                pdata = data;
                toolstruct->tool_on_key (pdata[0], pdata[1], pdata[2]);
                break;
            case TOOL_DRAWING:
                toolstruct->tool_drawing (data);
                break;
            default:
                break;
        }
    }
}
