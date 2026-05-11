#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <string.h>
#include <unistd.h>

#include "comic.h"
#include "doodle-treeview.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-pixmap.h"
#include "tbo-widget.h"
#include "tbo-window.h"

static void
drain_events (void)
{
    while (g_main_context_iteration (NULL, FALSE));
}

static gboolean
write_test_png (const gchar *path, gboolean has_alpha, gint width, gint height)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    gboolean ok;

    pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, width, height);
    if (pixbuf == NULL)
        return FALSE;

    gdk_pixbuf_fill (pixbuf, has_alpha ? 0x4477cc99 : 0x44aaeeff);
    ok = gdk_pixbuf_save (pixbuf, path, "png", &error, NULL);
    g_object_unref (pixbuf);

    if (!ok)
    {
        if (error != NULL)
            g_error_free (error);
        return FALSE;
    }

    return TRUE;
}

static GtkWidget *
find_widget_by_tooltip (GtkWidget *widget, const gchar *tooltip)
{
    GtkWidget *child;
    const gchar *current_tooltip = gtk_widget_get_tooltip_text (widget);

    if (current_tooltip != NULL && strcmp (current_tooltip, tooltip) == 0)
        return widget;

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
    {
        GtkWidget *found = find_widget_by_tooltip (child, tooltip);

        if (found != NULL)
            return found;
    }

    return NULL;
}

static GtkWidget *
find_search_entry (GtkWidget *widget)
{
    GtkWidget *child;

    if (GTK_IS_SEARCH_ENTRY (widget))
        return widget;

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
    {
        GtkWidget *found = find_search_entry (child);

        if (found != NULL)
            return found;
    }

    return NULL;
}

static void
expand_all_expanders (GtkWidget *widget)
{
    GtkWidget *child;

    if (GTK_IS_EXPANDER (widget))
        gtk_expander_set_expanded (GTK_EXPANDER (widget), TRUE);

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
        expand_all_expanders (child);
}

static GtkWidget *
find_asset_button_by_tooltip (GtkWidget *widget, const gchar *tooltip)
{
    GtkWidget *widget_with_tooltip = find_widget_by_tooltip (widget, tooltip);

    if (widget_with_tooltip != NULL && GTK_IS_BUTTON (widget_with_tooltip))
        return widget_with_tooltip;

    return NULL;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    GtkWidget *browser;
    GtkWidget *search;
    GtkWidget *asset_button;
    Frame *body_frame;
    Page *page;
    Frame *frame;
    TboObjectPixmap *pixmap;
    gchar *home_dir;
    gchar *xdg_data_home;
    gchar *doodle_dir;
    gchar *legacy_doodle_dir;
    gchar *body_doodle_dir;
    gchar *legacy_body_doodle_dir;
    gchar *visible_png;
    gchar *hidden_file;
    gchar *legacy_visible_png;
    gchar *legacy_hidden_file;
    gchar *body_visible_png;
    gchar *legacy_body_visible_png;
    gchar *save_path;
    gchar *contents = NULL;
    GtkWidget *body_asset_button;
    gint fd;
    gint preview_width = 0;
    gint preview_height = 0;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);

    home_dir = g_dir_make_tmp ("tbo-doodle-raster-XXXXXX", NULL);
    if (home_dir == NULL)
        return 2;

    xdg_data_home = g_build_filename (home_dir, ".local", "share", NULL);
    doodle_dir = g_build_filename (xdg_data_home, "tbo", "doodle", "catalog", NULL);
    legacy_doodle_dir = g_build_filename (home_dir, ".tbo", "doodle", "catalog", NULL);
    body_doodle_dir = g_build_filename (xdg_data_home, "tbo", "doodle", "zorrupe", "body", NULL);
    legacy_body_doodle_dir = g_build_filename (home_dir, ".tbo", "doodle", "zorrupe", "body", NULL);
    visible_png = g_build_filename (doodle_dir, "zz-raster-visible.png", NULL);
    hidden_file = g_build_filename (doodle_dir, "zz-raster-hidden.txt", NULL);
    legacy_visible_png = g_build_filename (legacy_doodle_dir, "zz-raster-visible.png", NULL);
    legacy_hidden_file = g_build_filename (legacy_doodle_dir, "zz-raster-hidden.txt", NULL);
    body_visible_png = g_build_filename (body_doodle_dir, "zz-body-preview.png", NULL);
    legacy_body_visible_png = g_build_filename (legacy_body_doodle_dir, "zz-body-preview.png", NULL);
    if (g_mkdir_with_parents (doodle_dir, 0700) != 0)
        return 3;
    if (g_mkdir_with_parents (legacy_doodle_dir, 0700) != 0)
        return 4;
    if (g_mkdir_with_parents (body_doodle_dir, 0700) != 0)
        return 5;
    if (g_mkdir_with_parents (legacy_body_doodle_dir, 0700) != 0)
        return 6;
    if (!write_test_png (visible_png, TRUE, 8, 8))
        return 7;
    if (!write_test_png (legacy_visible_png, TRUE, 8, 8))
        return 8;
    if (!g_file_set_contents (hidden_file, "not an image", -1, NULL))
        return 9;
    if (!g_file_set_contents (legacy_hidden_file, "not an image", -1, NULL))
        return 10;
    if (!write_test_png (body_visible_png, TRUE, 1024, 1024))
        return 11;
    if (!write_test_png (legacy_body_visible_png, TRUE, 1024, 1024))
        return 12;

    g_setenv ("HOME", home_dir, TRUE);
    g_setenv ("XDG_DATA_HOME", xdg_data_home, TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.doodlerastercatalog", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 13;

    tbo = tbo_new_tbo (app, 800, 450);
    browser = doodle_setup_tree (tbo, FALSE);
    tbo_widget_add_child (tbo->toolarea, browser);
    tbo_widget_show_all (browser);

    search = find_search_entry (browser);
    if (search == NULL)
        return 14;

    gtk_editable_set_text (GTK_EDITABLE (search), "zz body preview");
    drain_events ();
    body_asset_button = find_asset_button_by_tooltip (browser, "zorrupe/body/zz-body-preview.png");
    if (body_asset_button == NULL)
        return 15;
    gtk_widget_get_size_request (body_asset_button, &preview_width, &preview_height);
    if (preview_width != 108 || preview_height != 108)
        return 16;

    page = tbo_comic_get_current_page (tbo->comic);
    body_frame = tbo_page_new_frame (page, 20, 20, 120, 90);
    tbo_window_enter_frame (tbo, body_frame);
    g_signal_emit_by_name (body_asset_button, "clicked");
    drain_events ();
    if (tbo_frame_object_count (body_frame) != 1)
        return 17;
    pixmap = TBO_OBJECT_PIXMAP (tbo_frame_get_objects (body_frame)->data);
    if (strcmp (pixmap->path->str, "zorrupe/body/zz-body-preview.png") != 0)
        return 18;
    if (TBO_OBJECT_BASE (pixmap)->width != 45 || TBO_OBJECT_BASE (pixmap)->height != 45)
        return 19;

    tbo_empty_tool_area (tbo);
    browser = doodle_setup_tree (tbo, FALSE);
    tbo_widget_add_child (tbo->toolarea, browser);
    tbo_widget_show_all (browser);
    expand_all_expanders (browser);
    drain_events ();

    asset_button = find_asset_button_by_tooltip (browser, "catalog/zz-raster-visible.png");
    if (asset_button == NULL)
        return 20;
    if (find_asset_button_by_tooltip (browser, "catalog/zz-raster-hidden.txt") != NULL)
        return 21;

    frame = tbo_page_new_frame (page, 20, 20, 120, 90);
    tbo_window_enter_frame (tbo, frame);
    if (tbo_frame_object_count (frame) != 0)
        return 22;

    g_signal_emit_by_name (asset_button, "clicked");
    drain_events ();
    if (tbo_frame_object_count (frame) != 1)
        return 23;
    if (!TBO_IS_OBJECT_PIXMAP (tbo_frame_get_objects (frame)->data))
        return 24;

    pixmap = TBO_OBJECT_PIXMAP (tbo_frame_get_objects (frame)->data);
    if (strcmp (pixmap->path->str, "catalog/zz-raster-visible.png") != 0)
        return 25;

    save_path = g_build_filename (g_get_tmp_dir (), "tbo-doodle-raster-save-XXXXXX.tbo", NULL);
    fd = g_mkstemp (save_path);
    if (fd < 0)
        return 26;
    close (fd);

    if (!tbo_comic_save (tbo, save_path))
        return 27;
    if (!g_file_get_contents (save_path, &contents, NULL, NULL))
        return 28;
    if (strstr (contents, "path=\"catalog/zz-raster-visible.png\"") == NULL)
        return 29;
    if (strstr (contents, visible_png) != NULL)
        return 30;
    if (strstr (contents, legacy_visible_png) != NULL)
        return 31;

    g_free (contents);
    g_remove (save_path);
    g_free (save_path);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    drain_events ();
    g_object_unref (app);

    g_remove (visible_png);
    g_remove (hidden_file);
    g_remove (legacy_visible_png);
    g_remove (legacy_hidden_file);
    g_remove (body_visible_png);
    g_remove (legacy_body_visible_png);
    g_rmdir (doodle_dir);
    g_rmdir (legacy_doodle_dir);
    g_rmdir (body_doodle_dir);
    g_rmdir (legacy_body_doodle_dir);
    g_free (visible_png);
    g_free (hidden_file);
    g_free (legacy_visible_png);
    g_free (legacy_hidden_file);
    g_free (body_visible_png);
    g_free (legacy_body_visible_png);
    g_free (doodle_dir);
    g_free (legacy_doodle_dir);
    g_free (body_doodle_dir);
    g_free (legacy_body_doodle_dir);

    doodle_dir = g_build_filename (xdg_data_home, "tbo", "doodle", "zorrupe", NULL);
    g_rmdir (doodle_dir);
    g_free (doodle_dir);
    doodle_dir = g_build_filename (xdg_data_home, "tbo", "doodle", NULL);
    g_rmdir (doodle_dir);
    g_free (doodle_dir);
    doodle_dir = g_build_filename (xdg_data_home, "tbo", NULL);
    g_rmdir (doodle_dir);
    g_free (doodle_dir);

    doodle_dir = g_build_filename (home_dir, ".local", "share", NULL);
    g_rmdir (doodle_dir);
    g_free (doodle_dir);
    doodle_dir = g_build_filename (home_dir, ".local", NULL);
    g_rmdir (doodle_dir);
    g_free (doodle_dir);

    doodle_dir = g_build_filename (home_dir, ".tbo", "doodle", "zorrupe", NULL);
    g_rmdir (doodle_dir);
    g_free (doodle_dir);
    doodle_dir = g_build_filename (home_dir, ".tbo", "doodle", NULL);
    g_rmdir (doodle_dir);
    g_free (doodle_dir);
    doodle_dir = g_build_filename (home_dir, ".tbo", NULL);
    g_rmdir (doodle_dir);
    g_free (doodle_dir);

    g_free (xdg_data_home);
    g_rmdir (home_dir);
    g_free (home_dir);
    return 0;
}
