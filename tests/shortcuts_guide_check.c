#include <gtk/gtk.h>
#include <string.h>

#include "tbo-toolbar.h"
#include "tbo-window.h"

static GtkWindow *
find_shortcuts_window (TboWindow *tbo)
{
    GListModel *toplevels;
    guint i;

    toplevels = gtk_window_get_toplevels ();
    for (i = 0; i < g_list_model_get_n_items (toplevels); i++)
    {
        GtkWindow *window = GTK_WINDOW (g_list_model_get_item (toplevels, i));

        if (window != GTK_WINDOW (tbo->window) &&
            gtk_window_get_transient_for (window) == GTK_WINDOW (tbo->window) &&
            g_strcmp0 (gtk_window_get_title (window), "Keyboard Shortcuts") == 0)
            return window;

        g_object_unref (window);
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

static GtkEventControllerKey *
find_key_controller (GtkWidget *widget)
{
    GListModel *controllers;
    guint i;

    controllers = gtk_widget_observe_controllers (widget);
    for (i = 0; i < g_list_model_get_n_items (controllers); i++)
    {
        GtkEventController *controller = g_list_model_get_item (controllers, i);

        if (GTK_IS_EVENT_CONTROLLER_KEY (controller))
        {
            g_object_unref (controllers);
            return GTK_EVENT_CONTROLLER_KEY (controller);
        }

        g_object_unref (controller);
    }

    g_object_unref (controllers);
    return NULL;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    GtkWindow *shortcuts;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.shortcutsguide", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);

    if (strstr (gtk_widget_get_tooltip_text (tbo->toolbar->button_save), "Ctrl+S") == NULL)
        return 3;
    if (strstr (gtk_widget_get_tooltip_text (GTK_WIDGET (tbo->toolbar->tool_buttons[TBO_TOOLBAR_SELECTOR])), "(S)") == NULL)
        return 4;
    if (strstr (gtk_widget_get_tooltip_text (tbo->toolbar->button_zoom_in), "(+)") == NULL)
        return 5;

    g_action_group_activate_action (G_ACTION_GROUP (tbo->window), "shortcuts", NULL);
    while (g_main_context_iteration (NULL, FALSE));

    shortcuts = find_shortcuts_window (tbo);
    if (shortcuts == NULL)
        return 6;
    if (!GTK_IS_HEADER_BAR (gtk_window_get_titlebar (shortcuts)))
        return 9;
    if (!widget_tree_contains_label (GTK_WIDGET (shortcuts), "Save Comic") ||
        !widget_tree_contains_label (GTK_WIDGET (shortcuts), "Ctrl+S") ||
        !widget_tree_contains_label (GTK_WIDGET (shortcuts), "Selector") ||
        !widget_tree_contains_label (GTK_WIDGET (shortcuts), "Esc"))
        return 7;

    g_action_group_activate_action (G_ACTION_GROUP (tbo->window), "shortcuts", NULL);
    while (g_main_context_iteration (NULL, FALSE));
    if (find_shortcuts_window (tbo) != shortcuts)
        return 8;

    {
        GtkEventControllerKey *controller = find_key_controller (GTK_WIDGET (shortcuts));
        gboolean handled = FALSE;

        if (controller == NULL)
            return 10;
        g_signal_emit_by_name (controller, "key-pressed", GDK_KEY_Escape, 0u, 0u, &handled);
        g_object_unref (controller);
    }
    while (g_main_context_iteration (NULL, FALSE));
    if (find_shortcuts_window (tbo) != NULL)
        return 11;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
