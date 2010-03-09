#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo.h>

#include "export.h"
#include "ui-drawing.h"


gboolean
tbo_export_to_png (TboWindow *tbo)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    int width = tbo->comic->width;
    int height = tbo->comic->height;
    char rpath[255];
    char *filename;
    GList *page_list;
    int i;
    gint response;

    GtkWidget *filechooser;
    filechooser = gtk_file_chooser_dialog_new (_("Export as"),
                                               GTK_WINDOW (tbo->window),
                                               GTK_FILE_CHOOSER_ACTION_SAVE,
                                               GTK_STOCK_CANCEL,
                                               GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_SAVE,
                                               GTK_RESPONSE_ACCEPT,
                                               NULL);

    response = gtk_dialog_run (GTK_DIALOG (filechooser));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooser));
        for (i=0, page_list = g_list_first (tbo->comic->pages); page_list; i++, page_list = page_list->next)
        {
            surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
            cr = cairo_create(surface);

            tbo_drawing_draw_page (cr, (Page *)page_list->data, width, height);

            snprintf (rpath, 255, "%s%02d.png", filename, i);
            cairo_surface_write_to_png (surface, rpath);

            cairo_destroy(cr);
            cairo_surface_destroy(surface);
        }
    }

    gtk_widget_destroy ((GtkWidget *) filechooser);

    return FALSE;
}
