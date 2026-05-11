#include <gtk/gtk.h>

#include "comic.h"
#include "tbo-drawing.h"

int
main (void)
{
    Comic *comic;
    GtkWidget *drawing_widget;
    TboDrawing *drawing;

    gtk_init ();

    comic = tbo_comic_new ("Test", 800, 600);
    if (comic == NULL)
        return 1;

    if (!G_IS_OBJECT (comic) || !TBO_IS_COMIC (comic))
        return 2;

    drawing_widget = tbo_drawing_new_with_params (comic);
    drawing = TBO_DRAWING (drawing_widget);
    if (tbo_drawing_get_comic (drawing) != comic)
        return 3;

    g_object_ref (comic);
    tbo_comic_free (comic);
    if (tbo_comic_len (comic) != 1)
        return 4;

    g_object_unref (comic);
    if (tbo_drawing_get_comic (drawing) != NULL)
        return 5;

    g_object_unref (drawing_widget);
    return 0;
}
