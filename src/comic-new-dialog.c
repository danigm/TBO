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


#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "comic-new-dialog.h"
#include "tbo-widget.h"
#include "tbo-window.h"

typedef struct
{
    GtkWidget *spin_w;
    GtkWidget *spin_h;
} TboComicTemplateControls;

static void
template_selected_cb (GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data)
{
    TboComicTemplateControls *controls = user_data;
    gint width;
    gint height;

    (void) pspec;

    tbo_comic_template_get_default_size (gtk_drop_down_get_selected (dropdown), &width, &height);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (controls->spin_w), width);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (controls->spin_h), height);
}

gboolean
tbo_comic_new_dialog (GtkWidget *widget, TboWindow *window)
{
    GtkWidget *dialog;
    GtkWidget *headerbar;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *actions;
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *spin_w;
    GtkWidget *spin_h;
    GtkWidget *dropdown;
    GtkAdjustment *adjustment;
    TboDialogRunData data;
    TboComicTemplateControls template_controls;
    gint default_width;
    gint default_height;
    const char *template_names[] = {
        _("Empty"),
        _("Comic Strip"),
        "A4",
        _("Storyboard"),
        NULL
    };

    int width;
    int height;
    TboComicTemplate template;

    tbo_comic_template_get_default_size (TBO_COMIC_TEMPLATE_EMPTY, &default_width, &default_height);

    dialog = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (dialog), _("New Comic"));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window->window));
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

    headerbar = gtk_header_bar_new ();
    gtk_header_bar_set_show_title_buttons (GTK_HEADER_BAR (headerbar), TRUE);
    gtk_window_set_titlebar (GTK_WINDOW (dialog), headerbar);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class (vbox, "tbo-dialog-content");
    gtk_widget_set_margin_start (vbox, 12);
    gtk_widget_set_margin_end (vbox, 12);
    gtk_widget_set_margin_top (vbox, 12);
    gtk_widget_set_margin_bottom (vbox, 12);
    tbo_widget_add_child (dialog, vbox);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    label = gtk_label_new (_("Template: "));
    gtk_widget_set_size_request (label, 80, -1);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_label_set_yalign (GTK_LABEL (label), 0.5);
    dropdown = gtk_drop_down_new_from_strings (template_names);
    gtk_drop_down_set_selected (GTK_DROP_DOWN (dropdown), TBO_COMIC_TEMPLATE_EMPTY);
    tbo_box_pack_start (hbox, label, FALSE, FALSE, 0);
    tbo_box_pack_start (hbox, dropdown, TRUE, TRUE, 0);
    tbo_widget_add_child (vbox, hbox);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    label = gtk_label_new (_("Width: "));
    gtk_widget_set_size_request (label, 60, -1);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_label_set_yalign (GTK_LABEL (label), 0.5);
    adjustment = gtk_adjustment_new (default_width, 0, 10000, 100, 100, 0);
    spin_w = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
    tbo_box_pack_start (hbox, label, FALSE, FALSE, 0);
    tbo_box_pack_start (hbox, spin_w, TRUE, TRUE, 0);
    tbo_widget_add_child (vbox, hbox);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    label = gtk_label_new (_("Height: "));
    gtk_widget_set_size_request (label, 60, -1);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_label_set_yalign (GTK_LABEL (label), 0.5);
    adjustment = gtk_adjustment_new (default_height, 0, 10000, 100, 100, 0);
    spin_h = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
    tbo_box_pack_start (hbox, label, FALSE, FALSE, 0);
    tbo_box_pack_start (hbox, spin_h, TRUE, TRUE, 0);
    tbo_widget_add_child (vbox, hbox);

    template_controls.spin_w = spin_w;
    template_controls.spin_h = spin_h;
    g_signal_connect (dropdown, "notify::selected", G_CALLBACK (template_selected_cb), &template_controls);

    actions = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign (actions, GTK_ALIGN_END);

    button = gtk_button_new_with_mnemonic (_("_Cancel"));
    g_object_set_data (G_OBJECT (button), "tbo-response", GINT_TO_POINTER (GTK_RESPONSE_REJECT));
    g_signal_connect (button, "clicked", G_CALLBACK (tbo_dialog_button_cb), dialog);
    tbo_widget_add_child (actions, button);

    button = gtk_button_new_with_mnemonic (_("_OK"));
    gtk_widget_add_css_class (button, "suggested-action");
    g_object_set_data (G_OBJECT (button), "tbo-response", GINT_TO_POINTER (GTK_RESPONSE_ACCEPT));
    g_signal_connect (button, "clicked", G_CALLBACK (tbo_dialog_button_cb), dialog);
    tbo_widget_add_child (actions, button);

    tbo_widget_add_child (vbox, actions);

    tbo_dialog_run_data_init (&data, GTK_RESPONSE_REJECT);
    g_signal_connect (dialog, "close-request", G_CALLBACK (tbo_dialog_close_request_cb), &data);
    tbo_dialog_run (GTK_WINDOW (dialog), &data);

    if (data.response == GTK_RESPONSE_ACCEPT)
    {
        width = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin_w));
        height = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin_h));
        template = gtk_drop_down_get_selected (GTK_DROP_DOWN (dropdown));
        tbo_new_tbo_with_template (gtk_window_get_application (GTK_WINDOW (window->window)), width, height, template);
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
    tbo_dialog_run_data_clear (&data);

    return FALSE;
}
