
#include <gtk/gtk.h>
#include <malloc.h>
#include "comic.h"
#include "page.h"
#include "frame.h"

Page *
tbo_page_new (Comic *comic)
{
    Page *new_page;
    new_page = malloc(sizeof(Page));
    new_page->comic = comic;
    new_page->frames = NULL;

    return new_page;
}

void tbo_page_free (Page *page)
{
    GList *f;

    for (f=page->frames; f; f = g_list_next(f))
    {
        tbo_frame_free ((Frame *) f->data);
    }

    g_list_free (page->frames);
    free (page);
}

Frame *
tbo_page_new_frame (Page *page, int x, int y,
                                int w, int h)
{
    Frame *frame;

    frame = tbo_frame_new (x, y, w, h);
    page->frames = g_list_append (page->frames, frame);

    return frame;
}

void
tbo_page_del_frame (Page *page, int nth)
{
    Frame *frame;

    frame = (Frame *) g_list_nth_data (page->frames, nth);
    page->frames = g_list_remove (page->frames, frame);
    tbo_frame_free (frame);
}

int
tbo_page_len (Page *page)
{
    return g_list_length (g_list_first (page->frames));
}

int
tbo_page_frame_index (Page *page)
{
    return g_list_position (g_list_first (page->frames),
            page->frames) + 1;
}

gboolean
tbo_page_frame_first (Page *page)
{
    if (tbo_page_frame_index (page) == 1)
        return TRUE;
    return FALSE;
}

gboolean
tbo_page_frame_last (Page *page)
{
    if (tbo_page_frame_index (page) == tbo_page_len (page))
        return TRUE;
    return FALSE;
}

Frame *
tbo_page_next_frame (Page *page)
{
    if (page->frames->next)
    {
        page->frames = page->frames->next;
        return tbo_page_get_current_frame (page);
    }
    return NULL;
}

Frame *
tbo_page_prev_frame (Page *page)
{
    if (page->frames->prev)
    {
        page->frames = page->frames->prev;
        return tbo_page_get_current_frame (page);
    }
    return NULL;
}

Frame *
tbo_page_get_current_frame (Page *page)
{
    return (Frame *)page->frames->data;
}

