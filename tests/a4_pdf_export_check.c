#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic-load.h"
#include "comic.h"
#include "export.h"
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

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    gchar *base;
    gchar *pdffile;
    gchar *tbofile;
    gchar *pdfinfo_output = NULL;
    Comic *loaded;
    gchar *pdfinfo_program;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.a4pdfexport", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo_with_template (app, 1240, 1754, TBO_COMIC_TEMPLATE_A4);
    if (tbo_comic_get_paper (tbo->comic) != TBO_COMIC_PAPER_A4)
        return 3;

    base = make_tmp_base ("tbo-a4-export-XXXXXX");
    if (!tbo_export_file (tbo, base, "pdf", 1240, 1754))
        return 4;

    pdffile = g_strdup_printf ("%s.pdf", base);
    pdfinfo_program = g_find_program_in_path ("pdfinfo");
    if (pdfinfo_program != NULL)
    {
        gchar *command = g_strdup_printf ("%s %s", pdfinfo_program, pdffile);

        if (!g_spawn_command_line_sync (command, &pdfinfo_output, NULL, NULL, NULL))
            return 5;
        if (g_strstr_len (pdfinfo_output, -1, "Page size:       595.276 x 841.89 pts (A4)") == NULL)
            return 6;

        g_free (command);
        g_free (pdfinfo_output);
        g_free (pdfinfo_program);
    }

    tbofile = g_strdup_printf ("%s.tbo", base);
    if (!tbo_comic_save (tbo, tbofile))
        return 7;
    loaded = tbo_comic_load (tbofile);
    if (loaded == NULL || tbo_comic_get_paper (loaded) != TBO_COMIC_PAPER_A4)
        return 8;

    tbo_comic_free (loaded);
    g_remove (pdffile);
    g_remove (tbofile);
    g_free (pdffile);
    g_free (tbofile);
    g_free (base);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
