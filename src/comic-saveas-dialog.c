#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "comic-saveas-dialog.h"
#include "tbo-window.h"
#include "comic.h"

gboolean
tbo_comic_saveas_dialog (GtkWidget *widget, TboWindow *window)
{
    gint response;
    GtkWidget *filechooser;
    char *filename;
    char buffer[255];

    filechooser = gtk_file_chooser_dialog_new (_("Save as"),
                                               GTK_WINDOW (window->window),
                                               GTK_FILE_CHOOSER_ACTION_SAVE,
                                               GTK_STOCK_CANCEL,
                                               GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_SAVE,
                                               GTK_RESPONSE_ACCEPT,
                                               NULL);

    snprintf (buffer, 255, "%s.tbo", window->comic->title);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (filechooser), buffer);
    response = gtk_dialog_run (GTK_DIALOG (filechooser));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooser));
        tbo_comic_save (window->comic, filename);
    }

    gtk_widget_destroy ((GtkWidget *) filechooser);

    return FALSE;
}
