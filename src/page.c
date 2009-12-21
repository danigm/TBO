
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

