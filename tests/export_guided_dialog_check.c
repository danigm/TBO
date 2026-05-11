#include <gtk/gtk.h>
#include <string.h>

#include "comic.h"
#include "export.h"
#include "page.h"
#include "tbo-window.h"

typedef struct
{
    GtkApplication *app;
    TboWindow *tbo;
    gint status;
} DialogCheckState;

static void
drain_events (void)
{
    while (g_main_context_iteration (NULL, FALSE));
}

static GtkWindow *
find_export_dialog (DialogCheckState *state)
{
    GListModel *toplevels = gtk_window_get_toplevels ();
    guint i;

    for (i = 0; i < g_list_model_get_n_items (toplevels); i++)
    {
        GtkWindow *window = GTK_WINDOW (g_list_model_get_item (toplevels, i));

        if (window != GTK_WINDOW (state->tbo->window) &&
            gtk_window_get_transient_for (window) == GTK_WINDOW (state->tbo->window) &&
            g_strcmp0 (gtk_window_get_title (window), "Export") == 0)
            return window;

        g_object_unref (window);
    }

    return NULL;
}

static GtkWidget *
find_widget_by_name (GtkWidget *widget, const gchar *name)
{
    GtkWidget *child;

    if (g_strcmp0 (gtk_widget_get_name (widget), name) == 0)
        return widget;

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
    {
        GtkWidget *found = find_widget_by_name (child, name);

        if (found != NULL)
            return found;
    }

    return NULL;
}

static GtkWidget *
find_label_with_prefix (GtkWidget *widget, const gchar *prefix)
{
    GtkWidget *child;

    if (GTK_IS_LABEL (widget) && g_str_has_prefix (gtk_label_get_text (GTK_LABEL (widget)), prefix))
        return widget;

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
    {
        GtkWidget *found = find_label_with_prefix (child, prefix);

        if (found != NULL)
            return found;
    }

    return NULL;
}

static gboolean
inspect_export_dialog_cb (gpointer data)
{
    DialogCheckState *state = data;
    GtkWindow *dialog;
    GtkWidget *scope_dropdown;
    GtkWidget *range_from_spin;
    GtkWidget *range_to_spin;
    GtkWidget *preview_label;
    GtkWidget *preview_box;
    const gchar *label_text;

    dialog = find_export_dialog (state);
    if (dialog == NULL)
        return G_SOURCE_CONTINUE;

    scope_dropdown = find_widget_by_name (GTK_WIDGET (dialog), "export-scope");
    range_from_spin = find_widget_by_name (GTK_WIDGET (dialog), "export-range-from");
    range_to_spin = find_widget_by_name (GTK_WIDGET (dialog), "export-range-to");
    preview_label = find_label_with_prefix (GTK_WIDGET (dialog), "Preview:");
    preview_box = preview_label != NULL ? gtk_widget_get_next_sibling (preview_label) : NULL;
    if (scope_dropdown == NULL) { state->status = 30; gtk_window_close (dialog); return G_SOURCE_REMOVE; }
    if (range_from_spin == NULL) { state->status = 31; gtk_window_close (dialog); return G_SOURCE_REMOVE; }
    if (range_to_spin == NULL) { state->status = 32; gtk_window_close (dialog); return G_SOURCE_REMOVE; }
    if (preview_label == NULL) { state->status = 33; gtk_window_close (dialog); return G_SOURCE_REMOVE; }
    if (preview_box == NULL) { state->status = 34; gtk_window_close (dialog); return G_SOURCE_REMOVE; }

    if (!gtk_widget_is_sensitive (range_from_spin) ||
        gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (range_from_spin)) != 1 ||
        gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (range_to_spin)) != 3)
    {
        state->status = 4;
        gtk_window_close (dialog);
        return G_SOURCE_REMOVE;
    }

    label_text = gtk_label_get_text (GTK_LABEL (preview_label));
    if (strstr (label_text, "Page 1") == NULL || gtk_widget_get_first_child (preview_box) == NULL)
    {
        state->status = 5;
        gtk_window_close (dialog);
        return G_SOURCE_REMOVE;
    }

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (range_from_spin), 2);
    drain_events ();
    label_text = gtk_label_get_text (GTK_LABEL (preview_label));
    if (strstr (label_text, "Page 2") == NULL)
    {
        state->status = 6;
        gtk_window_close (dialog);
        return G_SOURCE_REMOVE;
    }

    gtk_drop_down_set_selected (GTK_DROP_DOWN (scope_dropdown), 1);
    drain_events ();
    label_text = gtk_label_get_text (GTK_LABEL (preview_label));
    if (strstr (label_text, "Current Page 3") == NULL || gtk_widget_is_sensitive (range_from_spin))
    {
        state->status = 7;
        gtk_window_close (dialog);
        return G_SOURCE_REMOVE;
    }

    state->status = 0;
    gtk_window_close (dialog);
    return G_SOURCE_REMOVE;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    DialogCheckState state;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.exportguideddialog", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_comic_new_page (tbo->comic);
    tbo_comic_new_page (tbo->comic);
    tbo_comic_set_current_page_nth (tbo->comic, 2);

    state.app = app;
    state.tbo = tbo;
    state.status = 99;
    g_idle_add (inspect_export_dialog_cb, &state);

    if (tbo_export (tbo))
        return 8;
    if (state.status != 0)
        return state.status;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    drain_events ();
    g_object_unref (app);
    return 0;
}
