/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "tbo-widget.h"

#define TBO_DIALOG_RUN_DATA_KEY "tbo-dialog-run-data"
#define TBO_ALERT_TEST_RESPONSE_NONE G_MININT

struct alert_run_data {
    GMainLoop *loop;
    gint response;
};

static gint alert_test_response = TBO_ALERT_TEST_RESPONSE_NONE;

static void
#if GTK_CHECK_VERSION(4, 10, 0)
alert_response_cb (GObject *source, GAsyncResult *result, gpointer user_data)
{
    GtkAlertDialog *dialog = GTK_ALERT_DIALOG (source);
    struct alert_run_data *data = user_data;
    GError *error = NULL;

    data->response = gtk_alert_dialog_choose_finish (dialog, result, &error);
    if (error != NULL)
    {
        g_error_free (error);
        data->response = -1;
    }
    g_main_loop_quit (data->loop);
}
#else
alert_response_cb (GtkButton *button, GtkWindow *dialog)
{
    tbo_dialog_button_cb (button, dialog);
}
#endif

GtkWidget *
tbo_widget_get_first_child (GtkWidget *widget)
{
    return gtk_widget_get_first_child (widget);
}

gint
tbo_widget_get_child_count (GtkWidget *widget)
{
    GtkWidget *child = gtk_widget_get_first_child (widget);
    gint count = 0;

    while (child != NULL)
    {
        count++;
        child = gtk_widget_get_next_sibling (child);
    }

    return count;
}

void
tbo_widget_add_child (GtkWidget *parent, GtkWidget *child)
{
    if (GTK_IS_WINDOW (parent))
        gtk_window_set_child (GTK_WINDOW (parent), child);
    else if (GTK_IS_SCROLLED_WINDOW (parent))
        gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (parent), child);
    else if (GTK_IS_BOX (parent))
        gtk_box_append (GTK_BOX (parent), child);
    else if (GTK_IS_BUTTON (parent))
        gtk_button_set_child (GTK_BUTTON (parent), child);
    else if (GTK_IS_EXPANDER (parent))
        gtk_expander_set_child (GTK_EXPANDER (parent), child);
    else if (GTK_IS_FRAME (parent))
        gtk_frame_set_child (GTK_FRAME (parent), child);
}

void
tbo_widget_remove_child (GtkWidget *parent, GtkWidget *child)
{
    if (GTK_IS_SCROLLED_WINDOW (parent))
        gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (parent), NULL);
    else if (GTK_IS_BOX (parent))
        gtk_box_remove (GTK_BOX (parent), child);
    else if (GTK_IS_BUTTON (parent))
        gtk_button_set_child (GTK_BUTTON (parent), NULL);
    else if (GTK_IS_EXPANDER (parent))
        gtk_expander_set_child (GTK_EXPANDER (parent), NULL);
    else if (GTK_IS_FRAME (parent))
        gtk_frame_set_child (GTK_FRAME (parent), NULL);
    else if (GTK_IS_WINDOW (parent))
        gtk_window_set_child (GTK_WINDOW (parent), NULL);
}

void
tbo_widget_destroy_all_children (GtkWidget *parent)
{
    GtkWidget *child;

    while ((child = gtk_widget_get_first_child (parent)) != NULL)
    {
        tbo_widget_remove_child (parent, child);
    }
}

void
tbo_box_pack_start (GtkWidget *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding)
{
    GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (box));

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        gtk_widget_set_hexpand (child, expand);
    else
        gtk_widget_set_vexpand (child, expand);

    gtk_widget_set_margin_start (child, padding);
    gtk_widget_set_margin_end (child, padding);
    gtk_widget_set_margin_top (child, padding);
    gtk_widget_set_margin_bottom (child, padding);
    gtk_box_append (GTK_BOX (box), child);
}

void
tbo_paned_pack_start (GtkWidget *paned, GtkWidget *child, gboolean resize, gboolean shrink)
{
    gtk_paned_set_start_child (GTK_PANED (paned), child);
    gtk_paned_set_resize_start_child (GTK_PANED (paned), resize);
    gtk_paned_set_shrink_start_child (GTK_PANED (paned), shrink);
}

void
tbo_paned_pack_end (GtkWidget *paned, GtkWidget *child, gboolean resize, gboolean shrink)
{
    gtk_paned_set_end_child (GTK_PANED (paned), child);
    gtk_paned_set_resize_end_child (GTK_PANED (paned), resize);
    gtk_paned_set_shrink_end_child (GTK_PANED (paned), shrink);
}

GtkWidget *
tbo_scrolled_window_get_child (GtkWidget *scrolled)
{
    GtkWidget *child = gtk_scrolled_window_get_child (GTK_SCROLLED_WINDOW (scrolled));

    if (GTK_IS_VIEWPORT (child))
        child = gtk_widget_get_first_child (child);

    return child;
}

void
tbo_scrolled_window_set_child (GtkWidget *scrolled, GtkWidget *child)
{
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), child);
}

void
tbo_dialog_run_data_init (TboDialogRunData *data, gint close_response)
{
    data->loop = g_main_loop_new (NULL, FALSE);
    data->response = GTK_RESPONSE_NONE;
    data->close_response = close_response;
}

void
tbo_dialog_run_data_clear (TboDialogRunData *data)
{
    if (data->loop != NULL)
    {
        g_main_loop_unref (data->loop);
        data->loop = NULL;
    }
}

gboolean
tbo_dialog_close_request_cb (GtkWindow *dialog, TboDialogRunData *data)
{
    if (data->response == GTK_RESPONSE_NONE)
        data->response = data->close_response;

    g_main_loop_quit (data->loop);
    return TRUE;
}

void
tbo_dialog_button_cb (GtkButton *button, GtkWindow *dialog)
{
    TboDialogRunData *data = g_object_get_data (G_OBJECT (dialog), TBO_DIALOG_RUN_DATA_KEY);
    gint response = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "tbo-response"));

    if (data != NULL)
        data->response = response;

    gtk_window_close (dialog);
}

gint
tbo_dialog_run (GtkWindow *dialog, TboDialogRunData *data)
{
    g_object_set_data (G_OBJECT (dialog), TBO_DIALOG_RUN_DATA_KEY, data);
    tbo_widget_show_all (GTK_WIDGET (dialog));
    gtk_window_present (dialog);
    g_main_loop_run (data->loop);
    return data->response;
}

gint
tbo_alert_choose (GtkWindow *parent,
                  const gchar *message,
                  const gchar *detail,
                  const gchar * const *buttons,
                  gint cancel_button,
                  gint default_button)
{
#if GTK_CHECK_VERSION(4, 10, 0)
    GtkAlertDialog *dialog;
    struct alert_run_data data;

    if (alert_test_response != TBO_ALERT_TEST_RESPONSE_NONE)
        return alert_test_response;

    dialog = gtk_alert_dialog_new ("%s", message);
    gtk_alert_dialog_set_detail (dialog, detail);
    gtk_alert_dialog_set_buttons (dialog, buttons);
    gtk_alert_dialog_set_cancel_button (dialog, cancel_button);
    gtk_alert_dialog_set_default_button (dialog, default_button);

    data.loop = g_main_loop_new (NULL, FALSE);
    data.response = -1;
    gtk_alert_dialog_choose (dialog, parent, NULL, alert_response_cb, &data);
    g_main_loop_run (data.loop);
    g_main_loop_unref (data.loop);
    g_object_unref (dialog);

    return data.response;
#else
    GtkWidget *dialog;
    GtkWidget *headerbar;
    GtkWidget *content;
    GtkWidget *label;
    GtkWidget *actions;
    TboDialogRunData data;
    gint response;

    if (alert_test_response != TBO_ALERT_TEST_RESPONSE_NONE)
        return alert_test_response;

    dialog = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (dialog), message);
    if (parent != NULL)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

    headerbar = gtk_header_bar_new ();
    gtk_header_bar_set_show_title_buttons (GTK_HEADER_BAR (headerbar), TRUE);
    gtk_window_set_titlebar (GTK_WINDOW (dialog), headerbar);

    content = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class (content, "tbo-dialog-content");
    gtk_widget_set_margin_start (content, 12);
    gtk_widget_set_margin_end (content, 12);
    gtk_widget_set_margin_top (content, 12);
    gtk_widget_set_margin_bottom (content, 12);
    tbo_widget_add_child (dialog, content);

    label = gtk_label_new (message);
    gtk_label_set_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_label_set_yalign (GTK_LABEL (label), 0.5);
    tbo_widget_add_child (content, label);

    if (detail != NULL && *detail != '\0')
    {
        label = gtk_label_new (detail);
        gtk_widget_add_css_class (label, "dim-label");
        gtk_label_set_wrap (GTK_LABEL (label), TRUE);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_label_set_yalign (GTK_LABEL (label), 0.5);
        tbo_widget_add_child (content, label);
    }

    actions = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign (actions, GTK_ALIGN_END);
    tbo_widget_add_child (content, actions);

    for (gint i = 0; buttons[i] != NULL; i++)
    {
        GtkWidget *button = gtk_button_new_with_label (buttons[i]);

        if (i == default_button)
            gtk_widget_add_css_class (button, "suggested-action");
        g_object_set_data (G_OBJECT (button), "tbo-response", GINT_TO_POINTER (i));
        g_signal_connect (button, "clicked", G_CALLBACK (alert_response_cb), dialog);
        tbo_widget_add_child (actions, button);
    }

    tbo_dialog_run_data_init (&data, cancel_button);
    g_signal_connect (dialog, "close-request", G_CALLBACK (tbo_dialog_close_request_cb), &data);
    response = tbo_dialog_run (GTK_WINDOW (dialog), &data);
    gtk_window_destroy (GTK_WINDOW (dialog));
    tbo_dialog_run_data_clear (&data);

    return response;
#endif
}

void
tbo_alert_show (GtkWindow *parent, const gchar *message, const gchar *detail)
{
    static const gchar *buttons[] = {"Close", NULL};

    tbo_alert_choose (parent, message, detail, buttons, 0, 0);
}

void
tbo_alert_set_test_response (gint response)
{
    alert_test_response = response;
}

void
tbo_alert_clear_test_response (void)
{
    alert_test_response = TBO_ALERT_TEST_RESPONSE_NONE;
}

void
tbo_widget_show_all (GtkWidget *widget)
{
    GtkWidget *child;

    if (widget == NULL)
        return;

    if (GTK_IS_POPOVER (widget))
    {
        gtk_widget_set_visible (widget, FALSE);
        return;
    }

    gtk_widget_set_visible (widget, TRUE);

    for (child = gtk_widget_get_first_child (widget);
         child != NULL;
         child = gtk_widget_get_next_sibling (child))
    {
        tbo_widget_show_all (child);
    }
}

GtkWidget *
tbo_picture_new_for_pixbuf (GdkPixbuf *pixbuf)
{
    gchar *buffer = NULL;
    gsize size = 0;
    GBytes *bytes;
    GdkTexture *texture;
    GtkWidget *picture;
    GError *error = NULL;

    if (pixbuf == NULL)
        return gtk_picture_new ();

    if (!gdk_pixbuf_save_to_buffer (pixbuf, &buffer, &size, "png", &error, NULL))
    {
        if (error != NULL)
            g_error_free (error);
        return gtk_picture_new ();
    }

    bytes = g_bytes_new_take (buffer, size);
    texture = gdk_texture_new_from_bytes (bytes, &error);
    g_bytes_unref (bytes);
    if (texture == NULL)
    {
        if (error != NULL)
            g_error_free (error);
        return gtk_picture_new ();
    }

    picture = gtk_picture_new_for_paintable (GDK_PAINTABLE (texture));
    g_object_unref (texture);
    return picture;
}
