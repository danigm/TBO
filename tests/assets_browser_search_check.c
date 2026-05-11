#include <gtk/gtk.h>
#include <string.h>

#include "doodle-treeview.h"
#include "tbo-widget.h"
#include "tbo-window.h"

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

static gboolean
widget_tree_contains_label (GtkWidget *widget, const gchar *text)
{
    GtkWidget *child;

    if (GTK_IS_LABEL (widget) && strstr (gtk_label_get_text (GTK_LABEL (widget)), text) != NULL)
        return TRUE;

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
    {
        if (widget_tree_contains_label (child, text))
            return TRUE;
    }

    return FALSE;
}

static gint
count_asset_buttons (GtkWidget *widget)
{
    GtkWidget *child;
    gint count = 0;

    if (g_object_get_data (G_OBJECT (widget), "tbo-asset-full-path") != NULL)
        count++;

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
        count += count_asset_buttons (child);

    return count;
}

static void
drain_events (void)
{
    while (g_main_context_iteration (NULL, FALSE));
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    GtkWidget *browser;
    GtkWidget *search;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.assetssearch", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    browser = doodle_setup_tree (tbo, FALSE);
    tbo_widget_add_child (tbo->toolarea, browser);
    tbo_widget_show_all (browser);

    search = find_search_entry (browser);
    if (search == NULL)
        return 3;
    if (g_strcmp0 (gtk_editable_get_text (GTK_EDITABLE (search)), "") != 0)
        return 4;
    if (!widget_tree_contains_label (browser, "Accesories (") &&
        !widget_tree_contains_label (browser, "Arcadia (") &&
        !widget_tree_contains_label (browser, "Doodle1 ("))
        return 5;

    gtk_editable_set_text (GTK_EDITABLE (search), "face smile big");
    drain_events ();
    if (!widget_tree_contains_label (browser, "Emotes (") || count_asset_buttons (browser) == 0)
        return 6;

    gtk_editable_set_text (GTK_EDITABLE (search), "zzznomatch");
    drain_events ();
    if (g_strcmp0 (gtk_editable_get_text (GTK_EDITABLE (search)), "zzznomatch") != 0)
        return 7;
    if (count_asset_buttons (browser) != 0)
        return 8;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    drain_events ();
    g_object_unref (app);
    return 0;
}
