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
#include <string.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include "comic.h"
#include "page.h"
#include "frame.h"
#include "tbo-list-utils.h"

struct _Page
{
    GObject parent_instance;

    GList *frames;
    Frame *current_frame;
};

struct _PageClass
{
    GObjectClass parent_class;
};

G_DEFINE_TYPE (Page, tbo_page, G_TYPE_OBJECT);

static void
tbo_page_dispose (GObject *object)
{
    Page *self = TBO_PAGE (object);

    self->current_frame = NULL;

    if (self->frames != NULL)
    {
        g_list_free_full (self->frames, (GDestroyNotify) tbo_frame_free);
        self->frames = NULL;
    }

    G_OBJECT_CLASS (tbo_page_parent_class)->dispose (object);
}

static void
tbo_page_init (Page *self)
{
    self->frames = NULL;
    self->current_frame = NULL;
}

static void
tbo_page_class_init (PageClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = tbo_page_dispose;
}

Page *
tbo_page_new (Comic *comic)
{
    (void) comic;
    return g_object_new (TBO_TYPE_PAGE, NULL);
}

void
tbo_page_free (Page *page)
{
    if (page != NULL)
        g_object_unref (page);
}

Page *
tbo_page_clone (Page *page)
{
    Page *new_page;
    GList *frames;
    Frame *selected_clone = NULL;

    if (page == NULL)
        return NULL;

    new_page = tbo_page_new (NULL);
    for (frames = page->frames; frames != NULL; frames = frames->next)
    {
        Frame *frame = TBO_FRAME (frames->data);
        Frame *cloned_frame = tbo_frame_clone (frame);

        tbo_page_add_frame (new_page, cloned_frame);
        if (page->current_frame == frame)
            selected_clone = cloned_frame;
    }

    if (selected_clone != NULL)
        tbo_page_set_current_frame (new_page, selected_clone);

    return new_page;
}

Frame *
tbo_page_new_frame (Page *page, int x, int y, int w, int h)
{
    Frame *frame;

    frame = tbo_frame_new (x, y, w, h);
    tbo_page_insert_frame (page, frame, -1);

    return frame;
}

void
tbo_page_add_frame (Page *page, Frame *frame)
{
    tbo_page_insert_frame (page, frame, -1);
}

void
tbo_page_insert_frame (Page *page, Frame *frame, int nth)
{
    tbo_current_list_insert (&page->frames, (gpointer *) &page->current_frame, frame, nth);
}

void
tbo_page_del_frame_by_index (Page *page, int nth)
{
    Frame *frame;

    frame = (Frame *) g_list_nth_data (page->frames, nth);
    tbo_page_del_frame (page, frame);
}

void
tbo_page_del_frame (Page *page, Frame *frame)
{
    GList *link;
    GList *next_link;
    GList *prev_link;

    if (frame == NULL)
        return;

    link = tbo_list_utils_link (page->frames, frame);
    if (link == NULL)
        return;

    next_link = link->next;
    prev_link = link->prev;
    if (!tbo_current_list_remove (&page->frames, (gpointer *) &page->current_frame, frame))
        return;

    if (page->current_frame == NULL && (next_link != NULL || prev_link != NULL))
        page->current_frame = (next_link != NULL ? next_link : prev_link)->data;

    tbo_frame_free (frame);
}

int
tbo_page_len (Page *page)
{
    return g_list_length (page->frames);
}

int
tbo_page_frame_index (Page *page)
{
    return tbo_current_list_index (page->frames, page->current_frame);
}

int
tbo_page_frame_position (Page *page)
{
    gint index = tbo_page_frame_index (page);

    return index >= 0 ? index + 1 : 0;
}

int
tbo_page_frame_nth (Page *page, Frame *frame)
{
    return tbo_current_list_index (page->frames, frame);
}

gboolean
tbo_page_frame_first (Page *page)
{
    if (tbo_page_frame_index (page) == 0)
        return TRUE;
    return FALSE;
}

gboolean
tbo_page_frame_last (Page *page)
{
    if (tbo_page_frame_index (page) == tbo_page_len (page) - 1)
        return TRUE;
    return FALSE;
}

Frame *
tbo_page_next_frame (Page *page)
{
    if (page->current_frame == NULL)
        return NULL;

    return tbo_current_list_next (page->frames, (gpointer *) &page->current_frame);
}

Frame *
tbo_page_prev_frame (Page *page)
{
    if (page->current_frame == NULL)
        return NULL;

    return tbo_current_list_prev (page->frames, (gpointer *) &page->current_frame);
}

Frame *
tbo_page_get_current_frame (Page *page)
{
    return page->current_frame;
}

void
tbo_page_set_current_frame (Page *page, Frame *frame)
{
    if (frame == NULL)
    {
        page->current_frame = NULL;
        return;
    }

    tbo_current_list_set (page->frames, (gpointer *) &page->current_frame, frame);
}

Frame *
tbo_page_first_frame (Page *page)
{
    return tbo_current_list_first (page->frames, (gpointer *) &page->current_frame);
}

GList *
tbo_page_get_frames (Page *page)
{
    return page->frames;
}

void
tbo_page_save (Page *page, FILE *file)
{
    char buffer[255];
    GList *f;

    snprintf (buffer, 255, " <page>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    for (f = page->frames; f; f = g_list_next (f))
    {
        tbo_frame_save ((Frame *) f->data, file);
    }

    snprintf (buffer, 255, " </page>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
}
