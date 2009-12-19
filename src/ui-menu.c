#include <stdio.h>
#include <gtk/gtk.h>

#include "ui-menu.h"

gboolean menu_handler (GtkWidget *widget, GdkEvent *event, gpointer data){
    printf ("Menu\n");
}

gboolean close_cb (GtkWidget *widget, GdkEvent *event, gpointer data){
    printf ("Ventana cerrada\n");
    gtk_main_quit ();
}

static const GtkActionEntry tbo_menu_entries [] = {
    /* Toplevel */

    { "File", NULL, "_File" },

    /* File menu */

    { "NewFile", GTK_STOCK_NEW, "_New", "<control>N",
      "Create a new file",
      G_CALLBACK (menu_handler) },

    { "OpenFile", GTK_STOCK_OPEN, "_Open", "<control>O",
      "Open a new file",
      G_CALLBACK (menu_handler) },

    { "SaveFile", GTK_STOCK_SAVE, "_Save", "<control>S",
      "Save current document",
      G_CALLBACK (menu_handler) },

    { "Quit", GTK_STOCK_QUIT, "_Quit", "<control>Q",
      "Quit",
      G_CALLBACK (close_cb) },
};

GtkWidget *generate_menu (GtkUIManager *manager, GtkWidget *window){
    GtkWidget *menu;
    GtkActionGroup *action_group;

    action_group = gtk_action_group_new ("MenuActions");
    gtk_action_group_add_actions (action_group, tbo_menu_entries,
                        G_N_ELEMENTS (tbo_menu_entries), window);

    gtk_ui_manager_insert_action_group (manager, action_group, 0);

    menu = gtk_ui_manager_get_widget (manager, "/menubar");
    
    return menu;
}
