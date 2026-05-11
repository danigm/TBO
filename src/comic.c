/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include "comic.h"
#include "tbo-types.h"
#include "tbo-window.h"
#include "page.h"
#include "comic-load.h"
#include "tbo-drawing.h"
#include "tbo-list-utils.h"
#include "tbo-utils.h"
#include "tbo-widget.h"

struct _Comic
{
    GObject parent_instance;

    char title[255];
    int width;
    int height;
    TboComicPaper paper;
    GList *pages;
    Page *current_page;
};

struct _ComicClass
{
    GObjectClass parent_class;
};

G_DEFINE_TYPE (Comic, tbo_comic, G_TYPE_OBJECT);

static void
tbo_comic_dispose (GObject *object)
{
    Comic *self = TBO_COMIC (object);

    self->current_page = NULL;

    if (self->pages != NULL)
    {
        g_list_free_full (self->pages, (GDestroyNotify) tbo_page_free);
        self->pages = NULL;
    }

    G_OBJECT_CLASS (tbo_comic_parent_class)->dispose (object);
}

static void
tbo_comic_init (Comic *self)
{
    self->title[0] = '\0';
    self->width = 0;
    self->height = 0;
    self->paper = TBO_COMIC_PAPER_NONE;
    self->pages = NULL;
    self->current_page = NULL;
}

static void
tbo_comic_class_init (ComicClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = tbo_comic_dispose;
}

Comic *
tbo_comic_new (const char *title, int width, int height)
{
    Comic *new_comic;

    new_comic = g_object_new (TBO_TYPE_COMIC, NULL);
    snprintf (new_comic->title, 255, "%s", title);
    new_comic->width = width;
    new_comic->height = height;
    tbo_comic_new_page (new_comic);

    return new_comic;
}

void
tbo_comic_free (Comic *comic)
{
    if (comic != NULL)
        g_object_unref (comic);
}

const gchar *
tbo_comic_get_title (Comic *comic)
{
    return comic->title;
}

gint
tbo_comic_get_width (Comic *comic)
{
    return comic->width;
}

gint
tbo_comic_get_height (Comic *comic)
{
    return comic->height;
}

TboComicPaper
tbo_comic_get_paper (Comic *comic)
{
    return comic->paper;
}

void
tbo_comic_set_paper (Comic *comic, TboComicPaper paper)
{
    comic->paper = paper;
}

gboolean
tbo_comic_get_pdf_page_size (Comic *comic, gdouble *width, gdouble *height)
{
    gdouble page_width;
    gdouble page_height;

    if (comic == NULL)
        return FALSE;

    switch (comic->paper)
    {
        case TBO_COMIC_PAPER_A4:
            page_width = 210.0 / 25.4 * 72.0;
            page_height = 297.0 / 25.4 * 72.0;
            break;
        case TBO_COMIC_PAPER_NONE:
        default:
            return FALSE;
    }

    if (width != NULL)
        *width = page_width;
    if (height != NULL)
        *height = page_height;

    return TRUE;
}

GList *
tbo_comic_get_pages (Comic *comic)
{
    return comic->pages;
}

Page *
tbo_comic_new_page (Comic *comic)
{
    Page *page;

    page = tbo_page_new (comic);
    tbo_comic_insert_page (comic, page, -1);

    return page;
}

void
tbo_comic_insert_page (Comic *comic, Page *page, int nth)
{
    tbo_current_list_insert (&comic->pages, (gpointer *) &comic->current_page, page, nth);
}

void
tbo_comic_del_page (Comic *comic, int nth)
{
    Page *page;
    GList *link;
    GList *next_link;
    GList *prev_link;

    page = (Page *) g_list_nth_data (comic->pages, nth);
    if (page == NULL)
        return;

    link = tbo_list_utils_link (comic->pages, page);
    next_link = link != NULL ? link->next : NULL;
    prev_link = link != NULL ? link->prev : NULL;

    if (!tbo_current_list_remove (&comic->pages, (gpointer *) &comic->current_page, page))
        return;

    if (comic->current_page == NULL && (next_link != NULL || prev_link != NULL))
        comic->current_page = (next_link != NULL ? next_link : prev_link)->data;

    tbo_page_free (page);
}

int
tbo_comic_len (Comic *comic)
{
    return g_list_length (comic->pages);
}

int
tbo_comic_page_index (Comic *comic)
{
    return tbo_current_list_index (comic->pages, comic->current_page);
}

int
tbo_comic_page_position (Comic *comic)
{
    gint index = tbo_comic_page_index (comic);

    return index >= 0 ? index + 1 : 0;
}

int
tbo_comic_page_nth (Comic *comic, Page *page)
{
    return tbo_current_list_index (comic->pages, page);
}

Page *
tbo_comic_next_page (Comic *comic)
{
    if (comic->current_page == NULL)
        return NULL;

    return tbo_current_list_next (comic->pages, (gpointer *) &comic->current_page);
}

Page *
tbo_comic_prev_page (Comic *comic)
{
    if (comic->current_page == NULL)
        return NULL;

    return tbo_current_list_prev (comic->pages, (gpointer *) &comic->current_page);
}

Page *
tbo_comic_get_current_page (Comic *comic)
{
    return comic->current_page;
}

void
tbo_comic_set_current_page (Comic *comic, Page *page)
{
    if (page == NULL)
    {
        comic->current_page = NULL;
        return;
    }

    tbo_current_list_set (comic->pages, (gpointer *) &comic->current_page, page);
}

void
tbo_comic_set_current_page_nth (Comic *comic, int nth)
{
    tbo_current_list_set_nth (comic->pages, (gpointer *) &comic->current_page, nth);
}

void
tbo_comic_reorder_page (Comic *comic, Page *page, int nth)
{
    gint old_index;

    if (comic == NULL || page == NULL)
        return;

    old_index = tbo_comic_page_nth (comic, page);
    if (old_index < 0 || old_index == nth)
        return;

    tbo_list_utils_remove (&comic->pages, page);
    tbo_list_utils_insert (&comic->pages, page, nth);
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

gboolean
save_comic_to_file (TboWindow *tbo,
                    const gchar *filename,
                    gboolean update_window_state,
                    gboolean mark_clean,
                    gboolean show_errors)
{
    GList *p;
    char buffer[255];
    char title[255] = {0};
    FILE *file = fopen (filename, "w");
    Comic *comic = tbo->comic;
    gboolean success;
    gint saved_errno = 0;

    if (!file)
    {
        if (show_errors)
        {
            perror (_("failed saving"));
            tbo_alert_show (GTK_WINDOW (tbo->window),
                            _("Failed saving"),
                            strerror (errno));
        }
        return FALSE;
    }

    if (update_window_state)
        get_base_name (filename, title, sizeof (title));

    if (comic->paper == TBO_COMIC_PAPER_A4)
        snprintf (buffer, 255, "<tbo width=\"%d\" height=\"%d\" paper=\"a4\">\n",
                  comic->width,
                  comic->height);
    else
        snprintf (buffer, 255, "<tbo width=\"%d\" height=\"%d\">\n",
                  comic->width,
                  comic->height);
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    for (p = comic->pages; p; p = g_list_next (p))
    {
        tbo_page_save ((Page *) p->data, file);
    }

    snprintf (buffer, 255, "</tbo>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    success = ferror (file) == 0;
    if (fclose (file) != 0)
        success = FALSE;

    if (!success)
    {
        saved_errno = errno != 0 ? errno : EIO;

        if (show_errors)
        {
            perror (_("failed saving"));
            tbo_alert_show (GTK_WINDOW (tbo->window),
                            _("Failed saving"),
                            strerror (saved_errno));
        }

        return FALSE;
    }

    if (update_window_state)
    {
        g_strlcpy (comic->title, title, sizeof (comic->title));
        gtk_window_set_title (GTK_WINDOW (tbo->window), comic->title);
    }

    if (mark_clean)
        tbo_window_mark_clean (tbo);

    return TRUE;
}

gboolean
tbo_comic_save (TboWindow *tbo, const gchar *filename)
{
    return save_comic_to_file (tbo, filename, TRUE, TRUE, TRUE);
}

gboolean
tbo_comic_save_snapshot (TboWindow *tbo, const gchar *filename)
{
    return save_comic_to_file (tbo, filename, FALSE, FALSE, FALSE);
}

gboolean
tbo_comic_open (TboWindow *window, const gchar *filename)
{
    Comic *newcomic = tbo_comic_load (filename);
    Comic *oldcomic;
    int nth;
    int n_pages;

    if (newcomic == NULL)
        return FALSE;

    tbo_window_reset_document_state (window);
    oldcomic = window->comic;

    n_pages = tbo_window_get_page_count (window);
    for (nth = n_pages - 1; nth >= 0; nth--)
    {
        tbo_window_remove_page_widget (window, nth);
    }

    window->comic = newcomic;
    gtk_window_set_title (GTK_WINDOW (window->window), tbo_comic_get_title (window->comic));
    tbo_comic_free (oldcomic);

    for (nth = 0; nth < tbo_comic_len (window->comic); nth++)
    {
        tbo_window_add_page_widget (window,
                                    create_darea (window),
                                    g_list_nth_data (tbo_comic_get_pages (window->comic), nth));
    }

    tbo_window_set_path (window, filename);
    tbo_window_set_current_tab_page (window, TRUE);
    tbo_drawing_adjust_scroll (TBO_DRAWING (window->drawing));
    tbo_drawing_update (TBO_DRAWING (window->drawing));
    tbo_window_refresh_status (window);
    tbo_window_mark_clean (window);
    return TRUE;
}
