#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
#include "dnd.h"
#include "export.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-base.h"
#include "tbo-object-pixmap.h"
#include "tbo-window.h"

static gboolean
write_test_png (const gchar *path, gboolean has_alpha, gint width, gint height)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    gboolean ok;

    pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, width, height);
    if (pixbuf == NULL)
        return FALSE;

    gdk_pixbuf_fill (pixbuf, has_alpha ? 0xaa553388 : 0x33aa55ff);
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

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    Frame *frame;
    GList *objects;
    gint pixmap_count = 0;
    gchar *tmpdir;
    gchar *rgba_png;
    gchar *rgb_png;
    gchar *export_base;
    gchar *export_png;
    GdkPixbuf *exported;
    TboObjectBase *rgba_asset;
    TboObjectBase *rgb_asset;
    gint fd;

    gtk_init ();

    tmpdir = g_dir_make_tmp ("tbo-raster-render-XXXXXX", NULL);
    if (tmpdir == NULL)
        return 2;

    rgba_png = g_build_filename (tmpdir, "rgba.png", NULL);
    rgb_png = g_build_filename (tmpdir, "rgb.png", NULL);
    if (!write_test_png (rgba_png, TRUE, 400, 400))
        return 3;
    if (!write_test_png (rgb_png, FALSE, 400, 200))
        return 4;

    app = gtk_application_new ("net.danigm.tbo.rasterrender", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 5;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 200, 140);
    tbo_window_enter_frame (tbo, frame);

    rgba_asset = tbo_dnd_insert_asset (tbo, rgba_png, 20, 20);
    if (rgba_asset == NULL)
        return 6;
    rgb_asset = tbo_dnd_insert_asset (tbo, rgb_png, 80, 20);
    if (rgb_asset == NULL)
        return 7;
    if (tbo_frame_object_count (frame) != 2)
        return 8;
    if (rgba_asset->width != 70 || rgba_asset->height != 70)
        return 9;
    if (rgb_asset->width != 140 || rgb_asset->height != 70)
        return 10;

    for (objects = tbo_frame_get_objects (frame); objects != NULL; objects = objects->next)
    {
        if (!TBO_IS_OBJECT_PIXMAP (objects->data))
            return 11;
        pixmap_count++;
    }
    if (pixmap_count != 2)
        return 12;

    export_base = g_build_filename (g_get_tmp_dir (), "tbo-raster-render-export-XXXXXX", NULL);
    fd = g_mkstemp (export_base);
    if (fd < 0)
        return 13;
    close (fd);
    g_remove (export_base);

    if (!tbo_export_file (tbo, export_base, "png", 800, 450))
        return 14;

    export_png = g_strdup_printf ("%s.png", export_base);
    if (!g_file_test (export_png, G_FILE_TEST_EXISTS))
        return 15;

    exported = gdk_pixbuf_new_from_file (export_png, NULL);
    if (exported == NULL)
        return 16;
    if (gdk_pixbuf_get_width (exported) != 800 || gdk_pixbuf_get_height (exported) != 450)
        return 17;

    g_object_unref (exported);
    g_remove (export_png);
    g_free (export_png);
    g_free (export_base);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);

    g_remove (rgba_png);
    g_remove (rgb_png);
    g_rmdir (tmpdir);
    g_free (rgba_png);
    g_free (rgb_png);
    g_free (tmpdir);
    return 0;
}
