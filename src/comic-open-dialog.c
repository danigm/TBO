#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "comic-open-dialog.h"
#include "ui-drawing.h"
#include "tbo-window.h"
#include "comic.h"

gboolean
tbo_comic_open_dialog (GtkWidget *widget, TboWindow *window)
{
    gint response;
    GtkWidget *filechooser;
    GtkFileFilter *filter;
    char *filename;
    char buffer[255];

    filechooser = gtk_file_chooser_dialog_new (_("Open"),
                                               GTK_WINDOW (window->window),
                                               GTK_FILE_CHOOSER_ACTION_OPEN,
                                               GTK_STOCK_CANCEL,
                                               GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_OPEN,
                                               GTK_RESPONSE_ACCEPT,
                                               NULL);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("TBO files"));
    gtk_file_filter_add_pattern (filter, "*.tbo");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (filechooser), filter);
    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (filechooser), filter);

    response = gtk_dialog_run (GTK_DIALOG (filechooser));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooser));
        tbo_comic_open (window, filename);
        update_drawing (window);
        tbo_window_update_status (window, 0, 0);
    }

    gtk_widget_destroy ((GtkWidget *) filechooser);

    return FALSE;
}
