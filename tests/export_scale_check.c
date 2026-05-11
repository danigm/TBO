#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>

#include "export.h"
#include "tbo-window.h"

typedef struct
{
    TboWindow *tbo;
    const gchar *filename;
} ExportDialogState;

static void
collect_export_controls (GtkWidget *widget,
                         GtkEntry **entry,
                         GtkSpinButton **width_spin,
                         GtkButton **save_button)
{
    GtkWidget *child;

    if (GTK_IS_ENTRY (widget) && *entry == NULL)
        *entry = GTK_ENTRY (widget);

    if (GTK_IS_SPIN_BUTTON (widget) && *width_spin == NULL)
        *width_spin = GTK_SPIN_BUTTON (widget);

    if (GTK_IS_BUTTON (widget) && gtk_widget_has_css_class (widget, "suggested-action"))
        *save_button = GTK_BUTTON (widget);

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
        collect_export_controls (child, entry, width_spin, save_button);
}

static GtkWindow *
find_export_dialog (ExportDialogState *state)
{
    GListModel *toplevels;
    guint i;

    toplevels = gtk_window_get_toplevels ();

    for (i = 0; i < g_list_model_get_n_items (toplevels); i++)
    {
        GtkWindow *window = GTK_WINDOW (g_list_model_get_item (toplevels, i));

        if (window != GTK_WINDOW (state->tbo->window) &&
            gtk_window_get_transient_for (window) == GTK_WINDOW (state->tbo->window))
            return window;

        g_object_unref (window);
    }

    return NULL;
}

static gboolean
respond_export_dialog (gpointer data)
{
    ExportDialogState *state = data;
    GtkWindow *dialog;
    GtkEntry *entry = NULL;
    GtkSpinButton *width_spin = NULL;
    GtkButton *save_button = NULL;

    dialog = find_export_dialog (state);
    if (dialog == NULL)
        return G_SOURCE_CONTINUE;

    collect_export_controls (GTK_WIDGET (dialog), &entry, &width_spin, &save_button);
    if (entry == NULL || width_spin == NULL || save_button == NULL)
        return G_SOURCE_CONTINUE;

    gtk_editable_set_text (GTK_EDITABLE (entry), state->filename);
    gtk_spin_button_set_value (width_spin, 1600);
    g_signal_emit_by_name (save_button, "clicked");
    g_object_unref (dialog);
    return G_SOURCE_REMOVE;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    gchar *tmpbase;
    gchar *pngfile;
    gint fd;
    gboolean exported;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    ExportDialogState state;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.exportscale", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    tmpbase = g_build_filename (g_get_tmp_dir (), "tbo-export-scale-XXXXXX", NULL);
    fd = g_mkstemp (tmpbase);
    if (fd < 0)
        return 3;
    close (fd);
    g_remove (tmpbase);

    state.tbo = tbo;
    state.filename = tmpbase;
    g_idle_add (respond_export_dialog, &state);

    exported = tbo_export (tbo);
    if (!exported)
        return 4;

    pngfile = g_strdup_printf ("%s.png", tmpbase);
    pixbuf = gdk_pixbuf_new_from_file (pngfile, &error);
    if (pixbuf == NULL)
        return 5;

    if (gdk_pixbuf_get_width (pixbuf) != 1600 || gdk_pixbuf_get_height (pixbuf) != 900)
        return 6;

    g_object_unref (pixbuf);
    g_remove (pngfile);
    g_free (pngfile);
    g_free (tmpbase);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
