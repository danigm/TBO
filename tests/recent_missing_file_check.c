#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "tbo-widget.h"
#include "tbo-window.h"

static gchar *
make_tmp_path (const gchar *pattern)
{
    gchar *path = g_build_filename (g_get_tmp_dir (), pattern, NULL);
    gint fd = g_mkstemp (path);

    if (fd >= 0)
        close (fd);

    return path;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    gchar *path;
    gchar **recent_projects;
    gsize n_projects = 0;

    gtk_init ();
    tbo_window_clear_persisted_state ();

    app = gtk_application_new ("net.danigm.tbo.recentmissingfile", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    path = make_tmp_path ("tbo-missing-recent-XXXXXX.tbo");
    tbo_window_add_recent_project (path);
    g_remove (path);

    tbo_alert_set_test_response (0);
    if (tbo_window_open_recent_project (tbo, path))
        return 3;
    tbo_alert_clear_test_response ();

    recent_projects = tbo_window_get_recent_projects (&n_projects);
    if (n_projects != 0)
        return 4;

    g_strfreev (recent_projects);
    g_free (path);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
