#include <stdio.h>
#include <gtk/gtk.h>
#include "ui-menu.h"
#include "ui-toolbar.h"
#include "config.h"
#include "comic-new-dialog.h"

gboolean toolbar_handler (GtkWidget *widget, GdkEvent *event, gpointer data){
    printf("toolbar\n");
}

static const GtkActionEntry tbo_tools_entries [] = {
    { "NewFileTool", GTK_STOCK_NEW, "_New", "<control>N",
      "Create a new file",
      G_CALLBACK (tbo_comic_new_dialog) },

    { "OpenFileTool", GTK_STOCK_OPEN, "_Open", "<control>O",
      "Open a new file",
      G_CALLBACK (toolbar_handler) },

    { "SaveFileTool", GTK_STOCK_SAVE, "_Save", "<control>S",
      "Save current document",
      G_CALLBACK (toolbar_handler) },
};

GtkWidget *generate_toolbar (GtkWidget *window){
    GtkWidget *toolbar;
    GtkActionGroup *action_group;
    GtkUIManager *manager;
    GError *error = NULL;

    manager = gtk_ui_manager_new ();
    gtk_ui_manager_add_ui_from_file (manager, DATA_DIR "/ui/tbo-toolbar-ui.xml", &error);
    if (error != NULL)
    {
        g_warning ("Could not merge tbo-toolbar-ui.xml: %s", error->message);
        g_error_free (error);
    }

    action_group = gtk_action_group_new ("ToolsActions");
    gtk_action_group_add_actions (action_group, tbo_tools_entries,
                        G_N_ELEMENTS (tbo_tools_entries), window);

    gtk_ui_manager_insert_action_group (manager, action_group, 0);

    toolbar = gtk_ui_manager_get_widget (manager, "/toolbar");
    
    return toolbar;
}

