#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "config.h"
#include "ui-menu.h"
#include "comic-new-dialog.h"
#include "comic-saveas-dialog.h"
#include "comic-open-dialog.h"
#include "tbo-window.h"

gboolean menu_handler (GtkWidget *widget, gpointer data){
    printf ("Menu :%s\n", ((TboWindow *) data)->comic->title);
    return FALSE;
}

gboolean close_cb (GtkWidget *widget, gpointer data){
    printf ("Ventana cerrada\n");
    tbo_window_free_cb (widget, NULL, ((TboWindow *) data));
    return FALSE;
}

static const GtkActionEntry tbo_menu_entries [] = {
    /* Toplevel */

    { "File", NULL, N_("_File") },

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

    { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
      N_("Quit"),
      G_CALLBACK (close_cb) },
};

GtkWidget *generate_menu (TboWindow *window){
    GtkWidget *menu;
    GtkActionGroup *action_group;
    GtkUIManager *manager;
    GError *error = NULL;

    manager = gtk_ui_manager_new ();
    gtk_ui_manager_add_ui_from_file (manager, DATA_DIR "/ui/tbo-menu-ui.xml", &error);
    if (error != NULL)
    {
        g_warning (_("Could not merge tbo-menu-ui.xml: %s"), error->message);
        g_error_free (error);
    }

    action_group = gtk_action_group_new ("MenuActions");
    gtk_action_group_set_translation_domain (action_group, NULL);
    gtk_action_group_add_actions (action_group, tbo_menu_entries,
                        G_N_ELEMENTS (tbo_menu_entries), window);

    gtk_ui_manager_insert_action_group (manager, action_group, 0);

    menu = gtk_ui_manager_get_widget (manager, "/menubar");
    
    return menu;
}

