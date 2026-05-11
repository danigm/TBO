#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
#include "export.h"
#include "frame.h"
#include "page.h"
#include "tbo-tool-selector.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

static gchar *
make_tmp_base (const gchar *pattern)
{
    gchar *path = g_build_filename (g_get_tmp_dir (), pattern, NULL);
    gint fd = g_mkstemp (path);

    if (fd >= 0)
        close (fd);
    g_remove (path);
    return path;
}

static GdkPixbuf *
load_pixbuf (const gchar *filename)
{
    return gdk_pixbuf_new_from_file (filename, NULL);
}

static gboolean
pixel_is_green (GdkPixbuf *pixbuf, gint x, gint y)
{
    guchar *pixel;

    pixel = gdk_pixbuf_get_pixels (pixbuf) +
            (y * gdk_pixbuf_get_rowstride (pixbuf)) +
            (x * gdk_pixbuf_get_n_channels (pixbuf));
    return pixel[1] > 140 && pixel[1] > pixel[0] + 60 && pixel[1] > pixel[2] + 60;
}

static gboolean
pixel_is_blue (GdkPixbuf *pixbuf, gint x, gint y)
{
    guchar *pixel;

    pixel = gdk_pixbuf_get_pixels (pixbuf) +
            (y * gdk_pixbuf_get_rowstride (pixbuf)) +
            (x * gdk_pixbuf_get_n_channels (pixbuf));
    return pixel[2] > 140 && pixel[2] > pixel[0] + 60 && pixel[2] > pixel[1] + 60;
}

static gboolean
pixel_is_white (GdkPixbuf *pixbuf, gint x, gint y)
{
    guchar *pixel;

    pixel = gdk_pixbuf_get_pixels (pixbuf) +
            (y * gdk_pixbuf_get_rowstride (pixbuf)) +
            (x * gdk_pixbuf_get_n_channels (pixbuf));
    return pixel[0] > 220 && pixel[1] > 220 && pixel[2] > 220;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolSelector *selector;
    Page *page1;
    Page *page2;
    Frame *selection_frame;
    Frame *page2_frame;
    GdkRGBA red = { 0.8, 0.2, 0.2, 1.0 };
    GdkRGBA blue = { 0.2, 0.2, 0.8, 1.0 };
    GdkRGBA green = { 0.2, 0.8, 0.2, 1.0 };
    gchar *page_base;
    gchar *selection_base;
    gchar *page_png;
    gchar *selection_png;
    gchar *numbered_page_png;
    GdkPixbuf *pixbuf;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.exportscope", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    page1 = tbo_comic_get_current_page (tbo->comic);

    tbo_frame_set_color_rgb (tbo_page_new_frame (page1, 10, 10, 140, 100), red.red, red.green, red.blue);
    selection_frame = tbo_page_new_frame (page1, 300, 50, 120, 80);
    tbo_frame_set_color_rgb (selection_frame, blue.red, blue.green, blue.blue);

    page2 = tbo_comic_new_page (tbo->comic);
    page2_frame = tbo_page_new_frame (page2, 20, 20, 160, 110);
    tbo_frame_set_color_rgb (page2_frame, green.red, green.green, green.blue);

    tbo_comic_set_current_page (tbo->comic, page2);
    page_base = make_tmp_base ("tbo-export-scope-page-XXXXXX");
    if (!tbo_export_file_with_scope (tbo, page_base, "png", 800, 450, TBO_EXPORT_SCOPE_CURRENT_PAGE))
        return 3;
    page_png = g_strdup_printf ("%s.png", page_base);
    numbered_page_png = g_strdup_printf ("%s0.png", page_base);
    if (!g_file_test (page_png, G_FILE_TEST_EXISTS))
        return 4;
    if (g_file_test (numbered_page_png, G_FILE_TEST_EXISTS))
        return 5;

    pixbuf = load_pixbuf (page_png);
    if (pixbuf == NULL)
        return 6;
    if (!pixel_is_green (pixbuf, 40, 40) || !pixel_is_white (pixbuf, 340, 70))
        return 7;
    g_object_unref (pixbuf);

    tbo_comic_set_current_page (tbo->comic, page1);
    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (selector, selection_frame);
    tbo_tool_selector_set_selected_obj (selector, NULL);

    selection_base = make_tmp_base ("tbo-export-scope-selection-XXXXXX");
    if (!tbo_export_file_with_scope (tbo, selection_base, "png", 240, 160, TBO_EXPORT_SCOPE_SELECTION))
        return 8;
    selection_png = g_strdup_printf ("%s.png", selection_base);
    if (!g_file_test (selection_png, G_FILE_TEST_EXISTS))
        return 9;

    pixbuf = load_pixbuf (selection_png);
    if (pixbuf == NULL)
        return 10;
    if (!pixel_is_blue (pixbuf,
                        gdk_pixbuf_get_width (pixbuf) / 2,
                        gdk_pixbuf_get_height (pixbuf) / 2))
        return 11;
    g_object_unref (pixbuf);

    g_remove (page_png);
    g_remove (selection_png);
    g_free (page_png);
    g_free (selection_png);
    g_free (numbered_page_png);
    g_free (page_base);
    g_free (selection_base);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
