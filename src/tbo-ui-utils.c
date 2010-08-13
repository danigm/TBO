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
