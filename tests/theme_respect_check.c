#include <gtk/gtk.h>

#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboWindow *tbo2;
    GtkSettings *settings;
    gboolean had_theme_name;
    gboolean had_prefer_dark;
    gchar *theme_name_after = NULL;
    gboolean prefer_dark_after = FALSE;

    gtk_init ();
    tbo_window_clear_persisted_state ();

    settings = gtk_settings_get_default ();
    if (settings == NULL)
        return 2;

    had_theme_name = g_object_class_find_property (G_OBJECT_GET_CLASS (settings), "gtk-theme-name") != NULL;
    had_prefer_dark = g_object_class_find_property (G_OBJECT_GET_CLASS (settings), "gtk-application-prefer-dark-theme") != NULL;

    if (had_theme_name)
        g_object_set (settings, "gtk-theme-name", "BlackMATE", NULL);
    if (had_prefer_dark)
        g_object_set (settings, "gtk-application-prefer-dark-theme", FALSE, NULL);

    app = gtk_application_new ("net.danigm.tbo.themerespect", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 3;

    tbo = tbo_new_tbo (app, 800, 450);

    if (had_theme_name)
        g_object_get (settings, "gtk-theme-name", &theme_name_after, NULL);
    if (had_prefer_dark)
        g_object_get (settings, "gtk-application-prefer-dark-theme", &prefer_dark_after, NULL);

    if (tbo_window_get_theme_mode () != TBO_THEME_MODE_SYSTEM)
        return 4;
    if (had_theme_name && g_strcmp0 (theme_name_after, "BlackMATE") != 0)
        return 5;
    if (had_prefer_dark && prefer_dark_after)
        return 6;
    if (gtk_widget_has_css_class (tbo->window, "dark"))
        return 7;
    if (gtk_widget_has_css_class (tbo->vbox, "dark"))
        return 8;

    g_action_group_change_action_state (G_ACTION_GROUP (tbo->window),
                                        "theme-mode",
                                        g_variant_new_string ("dark"));
    if (had_theme_name)
        g_object_get (settings, "gtk-theme-name", &theme_name_after, NULL);
    if (had_prefer_dark)
        g_object_get (settings, "gtk-application-prefer-dark-theme", &prefer_dark_after, NULL);
    if (tbo_window_get_theme_mode () != TBO_THEME_MODE_DARK)
        return 9;
    if (had_theme_name && g_strcmp0 (theme_name_after, "Adwaita") != 0)
        return 10;
    if (had_prefer_dark && !prefer_dark_after)
        return 11;

    tbo2 = tbo_new_tbo (app, 500, 400);
    if (had_prefer_dark)
        g_object_get (settings, "gtk-application-prefer-dark-theme", &prefer_dark_after, NULL);
    if (had_prefer_dark && !prefer_dark_after)
        return 12;

    g_action_group_change_action_state (G_ACTION_GROUP (tbo->window),
                                        "theme-mode",
                                        g_variant_new_string ("light"));
    if (had_prefer_dark)
        g_object_get (settings, "gtk-application-prefer-dark-theme", &prefer_dark_after, NULL);
    if (tbo_window_get_theme_mode () != TBO_THEME_MODE_LIGHT)
        return 13;
    if (had_prefer_dark && prefer_dark_after)
        return 14;

    g_action_group_change_action_state (G_ACTION_GROUP (tbo->window),
                                        "theme-mode",
                                        g_variant_new_string ("system"));
    if (had_theme_name)
        g_object_get (settings, "gtk-theme-name", &theme_name_after, NULL);
    if (had_prefer_dark)
        g_object_get (settings, "gtk-application-prefer-dark-theme", &prefer_dark_after, NULL);
    if (tbo_window_get_theme_mode () != TBO_THEME_MODE_SYSTEM)
        return 15;
    if (had_theme_name && g_strcmp0 (theme_name_after, "BlackMATE") != 0)
        return 16;
    if (had_prefer_dark && prefer_dark_after)
        return 17;

    g_free (theme_name_after);
    tbo_window_mark_clean (tbo);
    tbo_window_mark_clean (tbo2);
    gtk_window_close (GTK_WINDOW (tbo->window));
    gtk_window_close (GTK_WINDOW (tbo2->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
