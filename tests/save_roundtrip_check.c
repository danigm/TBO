#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "tbo-window.h"
#include "comic.h"
#include "comic-load.h"
#include "page.h"

int main(int argc, char **argv)
{
    GtkApplication *app;
    TboWindow *tbo;
    Comic *reloaded;
    Page *page;
    gchar *tmpname;
    gint fd;
    gint original_pages;
    gint reloaded_pages;
    gint original_frames;
    gint reloaded_frames;

    if (argc != 2)
        return 2;

    gtk_init();

    app = gtk_application_new ("net.danigm.tbo.saveroundtrip", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 3;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_comic_open (tbo, argv[1]);

    original_pages = tbo_comic_len (tbo->comic);
    page = tbo_comic_get_current_page (tbo->comic);
    original_frames = tbo_page_len (page);

    tmpname = g_build_filename (g_get_tmp_dir (), "tbo-roundtrip-XXXXXX.tbo", NULL);
    fd = g_mkstemp (tmpname);
    if (fd < 0)
        return 4;
    close (fd);

    if (!tbo_comic_save (tbo, tmpname))
        return 5;

    reloaded = tbo_comic_load (tmpname);
    if (reloaded == NULL)
        return 6;

    reloaded_pages = tbo_comic_len (reloaded);
    page = tbo_comic_get_current_page (reloaded);
    reloaded_frames = tbo_page_len (page);

    g_remove (tmpname);
    g_free (tmpname);
    tbo_comic_free (reloaded);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);

    if (original_pages != reloaded_pages)
        return 7;
    if (original_frames != reloaded_frames)
        return 8;

    return 0;
}
