#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <malloc.h>
#include <string.h>
#include "comic.h"
#include "tbo-types.h"
#include "tbo-window.h"
#include "page.h"
#include "comic-load.h"
#include "tbo-utils.h"

Comic *
tbo_comic_new (const char *title, int width, int height)
{
    Comic *new_comic;

    new_comic = malloc(sizeof(Comic));
    snprintf (new_comic->title, 255, "%s", title);
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

    for (p=g_list_first (comic->pages); p; p = g_list_next(p))
    {
        tbo_page_free ((Page *) p->data);
    }

    g_list_free (g_list_first (comic->pages));
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

    page = (Page *) g_list_nth_data (g_list_first (comic->pages), nth);
    comic->pages = g_list_remove (g_list_first (comic->pages), page);
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
    return g_list_position ( g_list_first (comic->pages), comic->pages);
}

int
tbo_comic_page_nth (Comic *comic, Page *page)
{
    return g_list_index (g_list_first (comic->pages), page);
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

void
tbo_comic_set_current_page (Comic *comic, Page *page)
{
    comic->pages = g_list_find (g_list_first (comic->pages), page);
}

void
tbo_comic_set_current_page_nth (Comic *comic, int nth)
{
    comic->pages = g_list_nth (g_list_first (comic->pages), nth);
}

gboolean
tbo_comic_page_first (Comic *comic)
{
    if (tbo_comic_page_index (comic) == 0)
        return TRUE;
    return FALSE;
}

gboolean
tbo_comic_page_last (Comic *comic)
{
    if (tbo_comic_page_index (comic) == tbo_comic_len (comic) - 1)
        return TRUE;
    return FALSE;
}

gboolean
tbo_comic_del_current_page (Comic *comic)
{
    int nth;
    Page *page;

    if (tbo_comic_len (comic) == 1)
        return FALSE;
    nth = tbo_comic_page_index (comic);

    page = tbo_comic_next_page (comic);
    if (page == NULL)
        page = tbo_comic_prev_page (comic);
    tbo_comic_del_page (comic, nth);
    tbo_comic_set_current_page (comic, page);
    return TRUE;
}

void
tbo_comic_save (TboWindow *tbo, char *filename)
{
    GList *p;
    char buffer[255];
    FILE *file = fopen (filename, "w");
    Comic *comic = tbo->comic;

    if (!file)
    {
        snprintf (buffer, 255, _("Failed saving: %s"), strerror (errno));
        GtkWidget *dialog = gtk_message_dialog_new (NULL,
                                                    GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE,
                                                    buffer);
        perror (_("failed saving"));
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy ((GtkWidget *) dialog);
        return;
    }
    get_base_name (filename, comic->title, 255);
    gtk_window_set_title (GTK_WINDOW (tbo->window), comic->title);

    snprintf (buffer, 255, "<tbo width=\"%d\" height=\"%d\">\n",
                                                    comic->width,
                                                    comic->height);
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    for (p=g_list_first (comic->pages); p; p = g_list_next(p))
    {
        tbo_page_save ((Page *) p->data, file);
    }

    snprintf (buffer, 255, "</tbo>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
    fclose (file);
}

void
tbo_comic_open (TboWindow *window, char *filename)
{
    Comic *newcomic = tbo_comic_load (filename);
    int nth;
    if (newcomic)
    {
        tbo_comic_free (window->comic);
        window->comic = newcomic;
        gtk_window_set_title (GTK_WINDOW (window->window), window->comic->title);

        for (nth=gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook)); nth>0; nth--)
        {
            gtk_notebook_remove_page (GTK_NOTEBOOK (window->notebook), nth);
        }

        for (nth=1; nth<tbo_comic_len (window->comic); nth++)
        {
            gtk_notebook_insert_page (GTK_NOTEBOOK (window->notebook),
                                      create_darea (window),
                                      NULL,
                                      nth);
        }
    }
}
