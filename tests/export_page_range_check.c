#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
#include "export.h"
#include "frame.h"
#include "page.h"
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
pixel_is_red (GdkPixbuf *pixbuf, gint x, gint y)
{
    guchar *pixel = gdk_pixbuf_get_pixels (pixbuf) +
                    (y * gdk_pixbuf_get_rowstride (pixbuf)) +
                    (x * gdk_pixbuf_get_n_channels (pixbuf));
    return pixel[0] > 140 && pixel[0] > pixel[1] + 60 && pixel[0] > pixel[2] + 60;
}

static gboolean
pixel_is_green (GdkPixbuf *pixbuf, gint x, gint y)
{
    guchar *pixel = gdk_pixbuf_get_pixels (pixbuf) +
                    (y * gdk_pixbuf_get_rowstride (pixbuf)) +
                    (x * gdk_pixbuf_get_n_channels (pixbuf));
    return pixel[1] > 140 && pixel[1] > pixel[0] + 60 && pixel[1] > pixel[2] + 60;
}

static gboolean
pixel_is_blue (GdkPixbuf *pixbuf, gint x, gint y)
{
    guchar *pixel = gdk_pixbuf_get_pixels (pixbuf) +
                    (y * gdk_pixbuf_get_rowstride (pixbuf)) +
                    (x * gdk_pixbuf_get_n_channels (pixbuf));
    return pixel[2] > 140 && pixel[2] > pixel[0] + 60 && pixel[2] > pixel[1] + 60;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page1;
    Page *page2;
    Page *page3;
    gchar *base;
    gchar *png0;
    gchar *png1;
    gchar *png2;
    GdkPixbuf *pixbuf;
    GdkRGBA red = { 0.8, 0.2, 0.2, 1.0 };
    GdkRGBA green = { 0.2, 0.8, 0.2, 1.0 };
    GdkRGBA blue = { 0.2, 0.2, 0.8, 1.0 };

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.exportpagerange", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page1 = tbo_comic_get_current_page (tbo->comic);
    tbo_frame_set_color_rgb (tbo_page_new_frame (page1, 10, 10, 120, 80), red.red, red.green, red.blue);

    page2 = tbo_comic_new_page (tbo->comic);
    tbo_frame_set_color_rgb (tbo_page_new_frame (page2, 10, 10, 120, 80), green.red, green.green, green.blue);

    page3 = tbo_comic_new_page (tbo->comic);
    tbo_frame_set_color_rgb (tbo_page_new_frame (page3, 10, 10, 120, 80), blue.red, blue.green, blue.blue);

    base = make_tmp_base ("tbo-export-range-XXXXXX");
    if (!tbo_export_file_with_scope_range (tbo, base, "png", 800, 450, TBO_EXPORT_SCOPE_ALL_PAGES, 2, 3))
        return 3;

    png0 = g_strdup_printf ("%s0.png", base);
    png1 = g_strdup_printf ("%s1.png", base);
    png2 = g_strdup_printf ("%s2.png", base);
    if (!g_file_test (png0, G_FILE_TEST_EXISTS) || !g_file_test (png1, G_FILE_TEST_EXISTS) || g_file_test (png2, G_FILE_TEST_EXISTS))
        return 4;

    pixbuf = load_pixbuf (png0);
    if (pixbuf == NULL || !pixel_is_green (pixbuf, 20, 20) || pixel_is_red (pixbuf, 20, 20))
        return 5;
    g_object_unref (pixbuf);

    pixbuf = load_pixbuf (png1);
    if (pixbuf == NULL || !pixel_is_blue (pixbuf, 20, 20) || pixel_is_green (pixbuf, 20, 20))
        return 6;
    g_object_unref (pixbuf);

    g_remove (png0);
    g_remove (png1);
    g_free (png0);
    g_free (png1);
    g_free (png2);
    g_free (base);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
