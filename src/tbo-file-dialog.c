/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <glib/gi18n.h>
#include "tbo-file-dialog.h"

struct file_dialog_data {
    GMainLoop *loop;
    GFile *file;
    GError *error;
};

static gchar *finish_dialog (struct file_dialog_data *data);

#if GTK_CHECK_VERSION(4, 10, 0)

static void
file_open_cb (GObject *source, GAsyncResult *result, gpointer user_data)
{
    GtkFileDialog *dialog = GTK_FILE_DIALOG (source);
    struct file_dialog_data *data = user_data;

    data->file = gtk_file_dialog_open_finish (dialog, result, &data->error);
    g_main_loop_quit (data->loop);
}

static void
file_save_cb (GObject *source, GAsyncResult *result, gpointer user_data)
{
    GtkFileDialog *dialog = GTK_FILE_DIALOG (source);
    struct file_dialog_data *data = user_data;

    data->file = gtk_file_dialog_save_finish (dialog, result, &data->error);
    g_main_loop_quit (data->loop);
}

static GtkFileDialog *
create_dialog (const gchar *title, TboWindow *window, const gchar *accept_label)
{
    GtkFileDialog *dialog = gtk_file_dialog_new ();

    gtk_file_dialog_set_title (dialog, title);
    gtk_file_dialog_set_modal (dialog, TRUE);
    gtk_file_dialog_set_accept_label (dialog, accept_label);

    return dialog;
}

static gchar *
run_open_dialog (GtkFileDialog *dialog, TboWindow *window)
{
    struct file_dialog_data data = {0};

    data.loop = g_main_loop_new (NULL, FALSE);
    gtk_file_dialog_open (dialog, GTK_WINDOW (window->window), NULL, file_open_cb, &data);
    g_main_loop_run (data.loop);
    return finish_dialog (&data);
}

static gchar *
run_save_dialog (GtkFileDialog *dialog, TboWindow *window)
{
    struct file_dialog_data data = {0};

    data.loop = g_main_loop_new (NULL, FALSE);
    gtk_file_dialog_save (dialog, GTK_WINDOW (window->window), NULL, file_save_cb, &data);
    g_main_loop_run (data.loop);
    return finish_dialog (&data);
}

static GListStore *
create_project_filters (void)
{
    GListStore *filters = g_list_store_new (GTK_TYPE_FILE_FILTER);
    GtkFileFilter *filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter, _("TBO files"));
    gtk_file_filter_add_pattern (filter, "*.tbo");
    g_list_store_append (filters, filter);
    g_object_unref (filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    g_list_store_append (filters, filter);
    g_object_unref (filter);

    return filters;
}

static void
set_initial_folder (GtkFileDialog *dialog, gchar *dirname)
{
    GFile *folder = g_file_new_for_path (dirname);

    gtk_file_dialog_set_initial_folder (dialog, folder);
    g_object_unref (folder);
    g_free (dirname);
}

#else

static void
file_response_cb (GtkNativeDialog *source, gint response, gpointer user_data)
{
    struct file_dialog_data *data = user_data;

    if (response == GTK_RESPONSE_ACCEPT)
        data->file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (source));
    g_main_loop_quit (data->loop);
}

static GtkFileChooserNative *
create_dialog (const gchar *title, TboWindow *window, const gchar *accept_label, GtkFileChooserAction action)
{
    return gtk_file_chooser_native_new (title,
                                        GTK_WINDOW (window->window),
                                        action,
                                        accept_label,
                                        _("_Cancel"));
}

static gchar *
run_dialog (GtkFileChooserNative *dialog)
{
    struct file_dialog_data data = {0};

    data.loop = g_main_loop_new (NULL, FALSE);
    g_signal_connect (dialog, "response", G_CALLBACK (file_response_cb), &data);
    gtk_native_dialog_show (GTK_NATIVE_DIALOG (dialog));
    g_main_loop_run (data.loop);
    return finish_dialog (&data);
}

static void
add_project_filters (GtkFileChooser *chooser)
{
    GtkFileFilter *filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter, _("TBO files"));
    gtk_file_filter_add_pattern (filter, "*.tbo");
    gtk_file_chooser_add_filter (chooser, filter);
    g_object_unref (filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (chooser, filter);
    g_object_unref (filter);
}

static void
add_image_filters (GtkFileChooser *chooser)
{
    GtkFileFilter *filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter, _("Image files"));
    gtk_file_filter_add_mime_type (filter, "image/*");
    gtk_file_chooser_add_filter (chooser, filter);
    g_object_unref (filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (chooser, filter);
    g_object_unref (filter);
}

static void
set_initial_folder (GtkFileChooserNative *dialog, gchar *dirname)
{
    GFile *folder = g_file_new_for_path (dirname);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), folder, NULL);
    g_object_unref (folder);
    g_free (dirname);
}

#endif

static gchar *
finish_dialog (struct file_dialog_data *data)
{
    gchar *path = NULL;

    if (data->file != NULL)
    {
        path = g_file_get_path (data->file);
        g_object_unref (data->file);
    }

    if (data->error != NULL)
        g_error_free (data->error);
    g_main_loop_unref (data->loop);
    return path;
}

gchar *
tbo_file_dialog_open_project (TboWindow *window)
{
#if GTK_CHECK_VERSION(4, 10, 0)
    GtkFileDialog *dialog = create_dialog (_("Open"), window, _("_Open"));
    GListStore *filters = create_project_filters ();
    gchar *path;

    gtk_file_dialog_set_filters (dialog, G_LIST_MODEL (filters));
    set_initial_folder (dialog, tbo_window_get_open_dir (window));
    path = run_open_dialog (dialog, window);

    g_object_unref (filters);
    g_object_unref (dialog);
    return path;
#else
    GtkFileChooserNative *dialog = create_dialog (_("Open"), window, _("_Open"), GTK_FILE_CHOOSER_ACTION_OPEN);
    gchar *path;

    add_project_filters (GTK_FILE_CHOOSER (dialog));
    set_initial_folder (dialog, tbo_window_get_open_dir (window));
    path = run_dialog (dialog);

    g_object_unref (dialog);
    return path;
#endif
}

gchar *
tbo_file_dialog_save_project (TboWindow *window, const gchar *suggested_name)
{
#if GTK_CHECK_VERSION(4, 10, 0)
    GtkFileDialog *dialog = create_dialog (_("Save As"), window, _("_Save"));
    GListStore *filters = create_project_filters ();
    gchar *path;

    gtk_file_dialog_set_filters (dialog, G_LIST_MODEL (filters));
    set_initial_folder (dialog, tbo_window_get_open_dir (window));
    if (suggested_name != NULL && *suggested_name != '\0')
        gtk_file_dialog_set_initial_name (dialog, suggested_name);
    path = run_save_dialog (dialog, window);

    g_object_unref (filters);
    g_object_unref (dialog);
    return path;
#else
    GtkFileChooserNative *dialog = create_dialog (_("Save As"), window, _("_Save"), GTK_FILE_CHOOSER_ACTION_SAVE);
    gchar *path;

    add_project_filters (GTK_FILE_CHOOSER (dialog));
    set_initial_folder (dialog, tbo_window_get_open_dir (window));
    if (suggested_name != NULL && *suggested_name != '\0')
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), suggested_name);
    path = run_dialog (dialog);

    g_object_unref (dialog);
    return path;
#endif
}

gchar *
tbo_file_dialog_open_image (TboWindow *window)
{
#if GTK_CHECK_VERSION(4, 10, 0)
    GtkFileDialog *dialog = create_dialog (_("Add Image"), window, _("_Open"));
    GListStore *filters = g_list_store_new (GTK_TYPE_FILE_FILTER);
    GtkFileFilter *filter = gtk_file_filter_new ();
    gchar *path;

    gtk_file_filter_set_name (filter, _("Image files"));
    gtk_file_filter_add_mime_type (filter, "image/*");
    g_list_store_append (filters, filter);
    g_object_unref (filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    g_list_store_append (filters, filter);
    g_object_unref (filter);

    gtk_file_dialog_set_filters (dialog, G_LIST_MODEL (filters));
    set_initial_folder (dialog, tbo_window_get_open_dir (window));
    path = run_open_dialog (dialog, window);

    g_object_unref (filters);
    g_object_unref (dialog);
    return path;
#else
    GtkFileChooserNative *dialog = create_dialog (_("Add Image"), window, _("_Open"), GTK_FILE_CHOOSER_ACTION_OPEN);
    gchar *path;

    add_image_filters (GTK_FILE_CHOOSER (dialog));
    set_initial_folder (dialog, tbo_window_get_open_dir (window));
    path = run_dialog (dialog);

    g_object_unref (dialog);
    return path;
#endif
}

gchar *
tbo_file_dialog_save_export (TboWindow *window, const gchar *current_text)
{
#if GTK_CHECK_VERSION(4, 10, 0)
    GtkFileDialog *dialog = create_dialog (_("Export"), window, _("_Save"));
    gchar *path;

    set_initial_folder (dialog, tbo_window_get_export_dir (window));

    if (current_text != NULL && *current_text != '\0')
    {
        if (g_path_is_absolute (current_text))
        {
            GFile *file = g_file_new_for_path (current_text);
            gtk_file_dialog_set_initial_file (dialog, file);
            g_object_unref (file);
        }
        else
        {
            gtk_file_dialog_set_initial_name (dialog, current_text);
        }
    }

    path = run_save_dialog (dialog, window);

    g_object_unref (dialog);
    return path;
#else
    GtkFileChooserNative *dialog = create_dialog (_("Export"), window, _("_Save"), GTK_FILE_CHOOSER_ACTION_SAVE);
    gchar *path;

    set_initial_folder (dialog, tbo_window_get_export_dir (window));

    if (current_text != NULL && *current_text != '\0')
    {
        if (g_path_is_absolute (current_text))
        {
            GFile *file = g_file_new_for_path (current_text);
            gtk_file_chooser_set_file (GTK_FILE_CHOOSER (dialog), file, NULL);
            g_object_unref (file);
        }
        else
        {
            gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), current_text);
        }
    }

    path = run_dialog (dialog);

    g_object_unref (dialog);
    return path;
#endif
}
