#include <gtk/gtk.h>
#include <cairo.h>

#include "comic-load.h"
#include "comic.h"
#include "page.h"
#include "tbo-drawing.h"

int main(int argc, char **argv)
{
    Comic *comic;
    Page *page;
    GtkWidget *drawing;
    cairo_surface_t *surface;
    cairo_t *cr;

    if (argc != 2)
        return 2;

    gtk_init();

    comic = tbo_comic_load (argv[1]);
    if (comic == NULL)
        return 3;

    if (tbo_comic_len (comic) <= 0)
        return 4;

    page = tbo_comic_get_current_page (comic);
    if (page == NULL || tbo_page_len (page) <= 0)
        return 5;

    drawing = tbo_drawing_new_with_params (comic);
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                          tbo_comic_get_width (comic),
                                          tbo_comic_get_height (comic));
    cr = cairo_create (surface);
    tbo_drawing_draw_page (TBO_DRAWING (drawing), cr, page,
                           tbo_comic_get_width (comic),
                           tbo_comic_get_height (comic));

    if (cairo_status (cr) != CAIRO_STATUS_SUCCESS ||
        cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_destroy (cr);
        cairo_surface_destroy (surface);
        tbo_comic_free (comic);
        return 6;
    }

    cairo_destroy (cr);
    cairo_surface_destroy (surface);
    tbo_comic_free (comic);
    return 0;
}
