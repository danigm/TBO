#include <stdio.h>
#include <gtk/gtk.h>
#include "ui-menu.h"
#include "ui-toolbar.h"

gboolean toolbar_handler (GtkWidget *widget, GdkEvent *event, gpointer data){
    printf("toolbar\n");
}

static const GtkActionEntry tbo_tools_entries [] = {
    { "NewFile", GTK_STOCK_NEW, "_New", "<control>N",
      "Create a new file",
      G_CALLBACK (toolbar_handler) },

    { "OpenFile", GTK_STOCK_OPEN, "_Open", "<control>O",
      "Open a new file",
      G_CALLBACK (toolbar_handler) },

    { "SaveFile", GTK_STOCK_SAVE, "_Save", "<control>S",
      "Save current document",
      G_CALLBACK (toolbar_handler) },
};

GtkWidget *generate_toolbar (GtkUIManager *manager, GtkWidget *window){
    GtkWidget *toolbar;
    GtkActionGroup *action_group;

    action_group = gtk_action_group_new ("ToolsActions");
    gtk_action_group_add_actions (action_group, tbo_tools_entries,
                        G_N_ELEMENTS (tbo_tools_entries), window);

    gtk_ui_manager_insert_action_group (manager, action_group, 0);

    toolbar = gtk_ui_manager_get_widget (manager, "/toolbar");
    
    return toolbar;
}
