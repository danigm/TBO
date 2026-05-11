#include <gtk/gtk.h>
#include <math.h>

#include "tbo-drawing.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboDrawing *drawing;
    gdouble zoom;
    gint i;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.zoombounds", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    drawing = TBO_DRAWING (tbo->drawing);

    for (i = 0; i < 100; i++)
        tbo_drawing_zoom_out (drawing);

    zoom = tbo_drawing_get_zoom (drawing);
    if (!isfinite (zoom) || zoom < ZOOM_STEP)
        return 3;

    tbo_drawing_zoom_fit (drawing);
    zoom = tbo_drawing_get_zoom (drawing);
    if (!isfinite (zoom) || zoom < ZOOM_STEP)
        return 4;

    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
