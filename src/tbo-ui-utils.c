#include <gtk/gtk.h>
#include "tbo-ui-utils.h"

GtkWidget *
add_spin_with_label (GtkWidget *container, const gchar *string, gint value)
{
        GtkWidget *label;
        GtkWidget *spin;
        GtkObject *adjustment;
        GtkWidget *hpanel;

        hpanel = gtk_hbox_new (FALSE, 0);
        label = gtk_label_new (string);
        gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
        adjustment = gtk_adjustment_new (value, 0, 10000, 1, 1, 0);
        spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
        gtk_box_pack_start (GTK_BOX (hpanel), label, TRUE, TRUE, 5);
        gtk_box_pack_start (GTK_BOX (hpanel), spin, FALSE, FALSE, 5);
        gtk_box_pack_start (GTK_BOX (container), hpanel, FALSE, FALSE, 5);

        return spin;
}
