
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
    comic->pages = g_list_append(comic->pages, page);

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
    return g_list_length (comic->pages);
}

