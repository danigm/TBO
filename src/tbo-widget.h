/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef __TBO_WIDGET_H__
#define __TBO_WIDGET_H__

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct
{
    GMainLoop *loop;
    gint response;
    gint close_response;
} TboDialogRunData;

GtkWidget *tbo_widget_get_first_child (GtkWidget *widget);
gint tbo_widget_get_child_count (GtkWidget *widget);
void tbo_widget_add_child (GtkWidget *parent, GtkWidget *child);
void tbo_widget_remove_child (GtkWidget *parent, GtkWidget *child);
void tbo_widget_destroy_all_children (GtkWidget *parent);
void tbo_box_pack_start (GtkWidget *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding);
void tbo_paned_pack_start (GtkWidget *paned, GtkWidget *child, gboolean resize, gboolean shrink);
void tbo_paned_pack_end (GtkWidget *paned, GtkWidget *child, gboolean resize, gboolean shrink);
GtkWidget *tbo_scrolled_window_get_child (GtkWidget *scrolled);
void tbo_scrolled_window_set_child (GtkWidget *scrolled, GtkWidget *child);
void tbo_dialog_run_data_init (TboDialogRunData *data, gint close_response);
void tbo_dialog_run_data_clear (TboDialogRunData *data);
gboolean tbo_dialog_close_request_cb (GtkWindow *dialog, TboDialogRunData *data);
void tbo_dialog_button_cb (GtkButton *button, GtkWindow *dialog);
gint tbo_dialog_run (GtkWindow *dialog, TboDialogRunData *data);
gint tbo_alert_choose (GtkWindow *parent,
                       const gchar *message,
                       const gchar *detail,
                       const gchar * const *buttons,
                       gint cancel_button,
                       gint default_button);
void tbo_alert_show (GtkWindow *parent, const gchar *message, const gchar *detail);
void tbo_alert_set_test_response (gint response);
void tbo_alert_clear_test_response (void);
void tbo_widget_show_all (GtkWidget *widget);
GtkWidget *tbo_picture_new_for_pixbuf (GdkPixbuf *pixbuf);

#endif
