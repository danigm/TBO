/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>
#include <string.h>

#include "export.h"
#include "tbo-drawing.h"
#include "tbo-ui-utils.h"
#include "tbo-types.h"

static int LOCK = 0;

struct export_spin_args {
    gint current_size;
    gint current_size2;
    GtkWidget *spin2;
    gdouble *scale;
};

static gboolean
export_size_cb (GtkWidget *widget, struct export_spin_args *args)
{
    if (!LOCK)
    {
        LOCK = 1;
        gint current_size = args->current_size;
        gint current_size2 = args->current_size2;
        gint new_size = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
        gint new_value;
        if (new_size)
        {
            *(args->scale) = new_size / (gdouble) current_size;
            new_value = (gint) (*(args->scale) * current_size2);
            gtk_spin_button_set_value (GTK_SPIN_BUTTON (args->spin2), new_value);
        }
        LOCK = 0;
    }
    return FALSE;
}

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
    return FALSE;
}

gboolean
tbo_export (TboWindow *tbo)
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
    gdouble scale = 1.0;
    gchar *export_to;
    gint export_to_index;
    struct export_spin_args spin_args;
    struct export_spin_args spin_args2;

    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *fileinput;
    GtkWidget *filelabel;
    GtkWidget *filebutton;
    GtkWidget *spinw;
    GtkWidget *spinh;
    GtkWidget *combobox;

    GtkWidget *button;

    dialog = gtk_dialog_new_with_buttons (_("Export as"),
                                            GTK_WINDOW (tbo->window),
                                            GTK_DIALOG_MODAL,
                                            GTK_STOCK_CANCEL,
                                            GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_SAVE,
                                            GTK_RESPONSE_ACCEPT,
                                            NULL);

    button = gtk_dialog_get_widget_for_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
    gtk_widget_grab_focus (GTK_WIDGET (button));

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

    spinw = add_spin_with_label (vbox, _("width: "), tbo->comic->width);
    spinh = add_spin_with_label (vbox, _("height: "), tbo->comic->height);

    spin_args.current_size = tbo->comic->width;
    spin_args.current_size2 = tbo->comic->height;
    spin_args.spin2 = spinh;
    spin_args.scale = &scale;
    g_signal_connect (spinw, "value-changed", G_CALLBACK (export_size_cb), &spin_args);

    spin_args2.current_size = tbo->comic->height;
    spin_args2.current_size2 = tbo->comic->width;
    spin_args2.spin2 = spinw;
    spin_args2.scale = &scale;
    g_signal_connect (spinh, "value-changed", G_CALLBACK (export_size_cb), &spin_args2);

    combobox = gtk_combo_box_new_text ();
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), _("guess by extension"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), ".png");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), ".pdf");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), ".svg");
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
    gtk_container_add (GTK_CONTAINER (vbox), combobox);

    gtk_widget_show_all (GTK_WIDGET (vbox));

    g_signal_connect (filebutton, "clicked", G_CALLBACK (filedialog_cb), fileinput);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        width = (gint) (width * scale);
        height = (gint) (height * scale);

        filename = (gchar *)gtk_entry_get_text (GTK_ENTRY (fileinput));
        /* 0 guess, 1 png, 2 pdf, 3 svg */
        export_to_index = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox));

        switch (export_to_index)
        {
            case 0:
                //guess
                if (strlen (filename) > 4)
                {
                    export_to = filename + strlen (filename) - 3;
                    filename = g_strndup (filename, strlen(filename) - 4);
                }
                else
                {
                    filename = g_strdup (filename);
                    export_to = "png";
                }
                break;
            case 1:
                export_to = "png";
                break;
            case 2:
                export_to = "pdf";
                break;
            case 3:
                export_to = "svg";
                break;
            default:
                export_to = "png";
                break;
        }

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
                surface = cairo_svg_surface_create (rpath, width, height);
                cr = cairo_create (surface);
            }
            // PNG or unknown format... default is png
            else
            {
                surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
                cr = cairo_create (surface);
            }

            cairo_scale (cr, scale, scale);

            // drawing the stuff
            tbo_drawing_draw_page (TBO_DRAWING (tbo->drawing), cr, (Page *)page_list->data, width/scale, height/scale);

            if (strcmp (export_to, "pdf") == 0)
                cairo_show_page (cr);
            else if (strcmp (export_to, "png") == 0)
                cairo_surface_write_to_png (surface, rpath);

            cairo_scale (cr, 1/scale, 1/scale);

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
    if (!export_to_index)
        g_free (filename);

    gtk_widget_destroy (GTK_WIDGET (dialog));

    return FALSE;
}
