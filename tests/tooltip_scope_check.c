#include <gtk/gtk.h>

#include "tbo-drawing.h"
#include "tbo-tooltip.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo1;
    TboWindow *tbo2;
    TboDrawing *drawing1;
    TboDrawing *drawing2;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.tooltipscope", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo1 = tbo_new_tbo (app, 800, 450);
    tbo2 = tbo_new_tbo (app, 800, 450);
    drawing1 = TBO_DRAWING (tbo1->drawing);
    drawing2 = TBO_DRAWING (tbo2->drawing);

    tbo_tooltip_set ("one", 10, 20, tbo1);
    if (drawing1->tooltip == NULL || strcmp (drawing1->tooltip->str, "one") != 0)
        return 3;
    if (drawing2->tooltip != NULL)
        return 4;

    tbo_tooltip_set_center_timeout ("two", 1000, tbo2);
    if (drawing2->tooltip == NULL || strcmp (drawing2->tooltip->str, "two") != 0)
        return 5;
    if (drawing2->tooltip_timeout_id == 0)
        return 6;
    if (drawing1->tooltip == NULL || strcmp (drawing1->tooltip->str, "one") != 0)
        return 7;

    tbo_tooltip_reset (tbo1);
    tbo_tooltip_reset (tbo2);
    if (drawing1->tooltip != NULL || drawing2->tooltip != NULL)
        return 8;
    if (drawing2->tooltip_timeout_id != 0)
        return 9;

    tbo_window_mark_clean (tbo1);
    tbo_window_mark_clean (tbo2);
    gtk_window_close (GTK_WINDOW (tbo1->window));
    gtk_window_close (GTK_WINDOW (tbo2->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
