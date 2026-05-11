#include <gtk/gtk.h>

#include "comic-load.h"
#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-text.h"

int
main (int argc, char **argv)
{
    Comic *comic;
    Page *page;
    GList *frames;
    GList *objects;
    Frame *frame;
    TboObjectText *text = NULL;
    GdkRGBA color;

    if (argc != 2)
        return 2;

    gtk_init ();

    comic = tbo_comic_load (argv[1]);
    if (comic == NULL)
        return 3;

    page = tbo_comic_get_current_page (comic);
    if (page == NULL)
        return 4;

    frames = tbo_page_get_frames (page);
    if (frames == NULL)
        return 5;

    frame = frames->data;
    tbo_frame_get_color (frame, &color);
    if (color.red < 0.99 || color.green < 0.99 || color.blue < 0.99)
        return 6;

    for (objects = tbo_frame_get_objects (frame); objects != NULL; objects = objects->next)
    {
        if (TBO_IS_OBJECT_TEXT (objects->data))
        {
            text = TBO_OBJECT_TEXT (objects->data);
            break;
        }
    }

    if (text == NULL)
        return 7;
    if (g_strcmp0 (tbo_object_text_get_text (text), "Tutorial") != 0)
        return 8;
    if (text->font_color->red > 0.01 || text->font_color->green > 0.01 || text->font_color->blue > 0.01)
        return 9;

    tbo_comic_free (comic);
    return 0;
}
