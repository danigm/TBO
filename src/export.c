#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>
#include <string.h>

#include "export.h"
#include "ui-drawing.h"


gboolean
tbo_export (TboWindow *tbo, const char *export_to)
{
    cairo_surface_t *surface = NULL;
    cairo_t *cr;
    int width = tbo->comic->width;
    int height = tbo->comic->height;
    char rpath[255];
    char format_pages[255];
    char *filename;
    GList *page_list;
    int i, n;
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
        n = g_list_length (tbo->comic->pages);
        for (i=0; n; n=n/10, i++);
        snprintf (format_pages, 255, "%%s%%0%dd.%%s", i);
        for (i=0, page_list = g_list_first (tbo->comic->pages); page_list; i++, page_list = page_list->next)
        {
            snprintf (rpath, 255, format_pages, filename, i, export_to);
            // PDF
            if (strcmp (export_to, "pdf") == 0)
            {
                if (!surface)
                {
                    snprintf (rpath, 255, "%s.%s", filename, export_to);
                    surface = cairo_pdf_surface_create (rpath, width, height);
                    cr = cairo_create (surface);
                }
            }
            // SVG
            else if (strcmp (export_to, "svg") == 0)
            {
                surface = cairo_svg_surface_create (rpath, height, width);
                cr = cairo_create (surface);
            }
            // PNG
            else if (strcmp (export_to, "png") == 0)
            {
                surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
                cr = cairo_create (surface);
            }

            // drawing the stuff
            tbo_drawing_draw_page (cr, (Page *)page_list->data, width, height);

            if (strcmp (export_to, "pdf") == 0)
                cairo_show_page (cr);
            else if (strcmp (export_to, "png") == 0)
                cairo_surface_write_to_png (surface, rpath);

            // Not destroying for multipage
            if (strcmp (export_to, "pdf") != 0)
            {
                cairo_surface_destroy (surface);
                cairo_destroy (cr);
                surface = NULL;
            }
        }

        if (surface)
        {
            cairo_surface_destroy (surface);
            cairo_destroy (cr);
        }
    }

    gtk_widget_destroy ((GtkWidget *) filechooser);

    return FALSE;
}
