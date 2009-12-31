#include <gtk/gtk.h>
#include <malloc.h>
#include <string.h>
#include "comic.h"
#include "page.h"

Comic *
tbo_comic_new (const char *title, int width, int height)
{
    Comic *new_comic;
    
    new_comic = malloc(sizeof(Comic));
    new_comic->title = malloc(strlen(title)*sizeof(char));
    sprintf (new_comic->title, "%s", title);
    new_comic->width = width;
    new_comic->height = height;
    new_comic->pages = NULL;
    tbo_comic_new_page (new_comic);

    return new_comic;
}

void
tbo_comic_free (Comic *comic)
{
    GList *p;

    for (p=comic->pages; p; p = g_list_next(p))
    {
        tbo_page_free ((Page *) p->data);
    }

    g_list_free (comic->pages);
    free (comic->title);
    free (comic);
}

Page *
tbo_comic_new_page (Comic *comic){
    Page *page;

    page = tbo_page_new (comic);
    comic->pages = g_list_append (comic->pages, page);

    return page;
}

void 
tbo_comic_del_page (Comic *comic, int nth)
{
    Page *page;

    page = (Page *) g_list_nth_data (comic->pages, nth);
    comic->pages = g_list_remove (comic->pages, page);
    tbo_page_free (page);
}

int
tbo_comic_len (Comic *comic)
{
    return g_list_length (g_list_first (comic->pages));
}

int
tbo_comic_page_index (Comic *comic)
{
    return g_list_position ( g_list_first (comic->pages), comic->pages) + 1;
}

Page *
tbo_comic_next_page (Comic *comic)
{
    if (comic->pages->next)
    {
        comic->pages = comic->pages->next;
        return tbo_comic_get_current_page (comic);
    }
    return NULL;
}

Page *
tbo_comic_prev_page (Comic *comic)
{
    if (comic->pages->prev)
    {
        comic->pages = comic->pages->prev;
        return tbo_comic_get_current_page (comic);
    }
    return NULL;
}

Page *
tbo_comic_get_current_page (Comic *comic)
{
    return (Page *)comic->pages->data;
}

gboolean
tbo_comic_page_first (Comic *comic)
{
    if (tbo_comic_page_index (comic) == 1)
        return TRUE;
    return FALSE;
}

gboolean
tbo_comic_page_last (Comic *comic)
{
    if (tbo_comic_page_index (comic) == tbo_comic_len (comic))
        return TRUE;
    return FALSE;
}

