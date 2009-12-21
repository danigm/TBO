#include <stdio.h>
#include <gtk/gtk.h>
#include "comic-new-dialog.h"

gboolean
tbo_comic_new_dialog (GtkWidget *widget, GtkWindow *window)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *spin;
    GtkObject *adjustment;
    gint response;

    dialog = gtk_dialog_new_with_buttons ("New Comic", 
                        GTK_WINDOW (window),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_OK, 
                        GTK_RESPONSE_ACCEPT,
                        GTK_STOCK_CANCEL,
                        GTK_RESPONSE_REJECT,
                        NULL);
    
    vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    
    hbox = gtk_hbox_new (FALSE, FALSE);
    label = gtk_label_new ("width: ");
    gtk_widget_set_size_request (label, 60, -1);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    adjustment = gtk_adjustment_new (800, 0, 10000, 100, 100, 0);
    spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), spin, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (vbox), hbox);

    hbox = gtk_hbox_new (FALSE, FALSE);
    label = gtk_label_new ("height: ");
    gtk_widget_set_size_request (label, 60, -1);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    adjustment = gtk_adjustment_new (500, 0, 10000, 100, 100, 0);
    spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), spin, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (vbox), hbox);

    gtk_widget_show_all (vbox);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        printf ("OK\n");
    }
    else
    {
        printf ("NOK\n");
    }

    gtk_widget_destroy ((GtkWidget *) dialog);

    return FALSE;
}

