#include "comic.h"
#include "page.h"
#include "frame.h"

int
main (void)
{
    Comic *comic = tbo_comic_new ("Test", 800, 600);
    Page *page1;
    Page *page2;
    Page *page3;
    Page *page4;
    Frame *frame1;
    Frame *frame2;
    Frame *frame3;

    if (comic == NULL)
        return 1;

    page1 = tbo_comic_get_current_page (comic);
    if (page1 == NULL)
        return 2;

    page2 = tbo_comic_new_page (comic);
    if (tbo_comic_get_current_page (comic) != page1)
        return 3;

    tbo_comic_set_current_page (comic, page2);
    page3 = tbo_comic_new_page (comic);
    if (tbo_comic_get_current_page (comic) != page2)
        return 4;

    if (tbo_comic_next_page (comic) != page3)
        return 5;
    if (tbo_comic_get_current_page (comic) != page3)
        return 6;

    tbo_comic_del_page (comic, 0);
    if (tbo_comic_get_current_page (comic) != page3)
        return 7;

    if (!tbo_comic_del_current_page (comic))
        return 8;
    if (tbo_comic_get_current_page (comic) != page2)
        return 9;

    page4 = tbo_comic_new_page (comic);
    if (tbo_comic_get_current_page (comic) != page2)
        return 10;

    frame1 = tbo_page_new_frame (page2, 0, 0, 20, 20);
    if (tbo_page_get_current_frame (page2) != frame1)
        return 11;

    frame2 = tbo_page_new_frame (page2, 10, 10, 30, 30);
    if (tbo_page_get_current_frame (page2) != frame1)
        return 12;

    tbo_page_set_current_frame (page2, frame2);
    frame3 = tbo_page_new_frame (page2, 20, 20, 40, 40);
    if (tbo_page_get_current_frame (page2) != frame2)
        return 13;

    tbo_page_del_frame (page2, frame1);
    if (tbo_page_get_current_frame (page2) != frame2)
        return 14;

    tbo_page_del_frame (page2, frame2);
    if (tbo_page_get_current_frame (page2) != frame3)
        return 15;

    tbo_page_set_current_frame (page2, frame3);
    if (tbo_page_frame_index (page2) != 0)
        return 16;

    if (tbo_comic_page_nth (comic, page4) != 1)
        return 17;

    tbo_comic_free (comic);
    return 0;
}
