#include <glib-object.h>

#include "comic.h"
#include "page.h"
#include "frame.h"

int
main (void)
{
    Comic *comic;
    Page *page;
    Frame *frame;

    comic = tbo_comic_new ("Test", 800, 600);
    if (comic == NULL)
        return 1;

    page = tbo_comic_get_current_page (comic);
    if (page == NULL)
        return 2;

    if (!G_IS_OBJECT (page) || !TBO_IS_PAGE (page))
        return 3;

    frame = tbo_page_new_frame (page, 10, 20, 30, 40);
    if (frame == NULL)
        return 4;

    g_object_ref (page);
    tbo_comic_free (comic);

    if (tbo_page_len (page) != 1)
        return 5;
    if (tbo_page_get_current_frame (page) != frame)
        return 6;
    if (tbo_frame_get_width (frame) != 30 || tbo_frame_get_height (frame) != 40)
        return 7;

    tbo_page_del_frame (page, frame);
    if (tbo_page_len (page) != 0)
        return 8;

    g_object_unref (page);
    return 0;
}
