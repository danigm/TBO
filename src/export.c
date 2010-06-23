#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>
#include <string.h>

#include "export.h"
#include "ui-drawing.h"


gboolean
filedialog_cb (GtkWidget *widget, gpointer data)
{
    gint response;
    gchar *filename;
    GtkWidget *filechooserdialog;
    GtkEntry *entry = GTK_ENTRY (data);

    filechooserdialog = gtk_file_chooser_dialog_new (_("Export as"),
                                                     NULL,
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     GTK_STOCK_CANCEL,
                                                     GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_SAVE,
                                                     GTK_RESPONSE_ACCEPT,
                                                     NULL);
    response = gtk_dialog_run (GTK_DIALOG (filechooserdialog));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
        gtk_entry_set_text (entry, filename);
    }

    gtk_widget_destroy (GTK_WIDGET (filechooserdialog));
}

gboolean
tbo_export (TboWindow *tbo, const gchar *export_to)
{
    cairo_surface_t *surface = NULL;
    cairo_t *cr;
    gint width = tbo->comic->width;
    gint height = tbo->comic->height;
    gchar rpath[255];
    gchar format_pages[255];
    gchar *filename;
    GList *page_list;
    gint i, n, n2;
    gint response;

    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *fileinput;
    GtkWidget *filelabel;
    GtkWidget *filebutton;

    dialog = gtk_dialog_new_with_buttons (_("Export as"),
                                            GTK_WINDOW (tbo->window),
                                            GTK_DIALOG_MODAL,
                                            GTK_STOCK_CANCEL,
                                            GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_SAVE,
                                            GTK_RESPONSE_ACCEPT,
                                            NULL);

    filebutton = gtk_button_new_from_stock (GTK_STOCK_OPEN);
    vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    hbox = gtk_hbox_new (FALSE, 5);
    filelabel = gtk_label_new (_("Filename: "));
    fileinput = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (fileinput), tbo->comic->title);
    gtk_container_add (GTK_CONTAINER (hbox), filelabel);
    gtk_container_add (GTK_CONTAINER (hbox), fileinput);
    gtk_container_add (GTK_CONTAINER (hbox), filebutton);
    gtk_container_add (GTK_CONTAINER (vbox), hbox);
    gtk_widget_show_all (GTK_WIDGET (vbox));

    g_signal_connect (filebutton, "clicked", G_CALLBACK (filedialog_cb), fileinput);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        filename = (gchar *)gtk_entry_get_text (GTK_ENTRY (fileinput));
        n = g_list_length (tbo->comic->pages);
        n2 = n;
        for (i=0; n; n=n/10, i++);
        snprintf (format_pages, 255, "%%s%%0%dd.%%s", i);
        for (i=0, page_list = g_list_first (tbo->comic->pages); page_list; i++, page_list = page_list->next)
        {
            snprintf (rpath, 255, format_pages, filename, i, export_to);
            if (n2 == 1)
                snprintf (rpath, 255, "%s.%s", filename, export_to);
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

    gtk_widget_destroy (GTK_WIDGET (dialog));

    return FALSE;
}
