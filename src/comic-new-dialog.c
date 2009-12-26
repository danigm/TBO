#include <stdio.h>
#include <gtk/gtk.h>
#include "comic-new-dialog.h"
#include "tbo-window.h"

gboolean
tbo_comic_new_dialog (GtkWidget *widget, TboWindow *window)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *spin_w;
    GtkWidget *spin_h;
    GtkObject *adjustment;
    gint response;

    int width;
    int height;

    dialog = gtk_dialog_new_with_buttons ("New Comic", 
                        GTK_WINDOW (window->window),
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
    spin_w = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), spin_w, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (vbox), hbox);

    hbox = gtk_hbox_new (FALSE, FALSE);
    label = gtk_label_new ("height: ");
    gtk_widget_set_size_request (label, 60, -1);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    adjustment = gtk_adjustment_new (500, 0, 10000, 100, 100, 0);
    spin_h = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), spin_h, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (vbox), hbox);

    gtk_widget_show_all (vbox);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        width = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin_w));
        height = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin_h));
        tbo_new_tbo (width, height);
    }
    else
    {
        printf ("NOK\n");
    }

    gtk_widget_destroy ((GtkWidget *) dialog);

    return FALSE;
}

