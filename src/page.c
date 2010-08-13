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

    if (tbo_page_len (page) > 0)
    {
        for (f=tbo_page_get_frames (page); f; f=g_list_next(f))
        {
            tbo_frame_free ((Frame *) f->data);
        }
    }
    g_list_free (tbo_page_get_frames (page));
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
tbo_page_add_frame (Page *page, Frame *frame)
{
    page->frames = g_list_append (page->frames, frame);
}

void
tbo_page_del_frame_by_index (Page *page, int nth)
{
    Frame *frame;

    frame = (Frame *) g_list_nth_data (g_list_first (page->frames), nth);
    tbo_page_del_frame (page, frame);
}

void
tbo_page_del_frame (Page *page, Frame *frame)
{
    page->frames = g_list_remove (g_list_first (page->frames), frame);
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

void
tbo_page_set_current_frame (Page *page, Frame *frame)
{
    page->frames = g_list_find (g_list_first (page->frames), frame);
}

Frame *
tbo_page_first_frame (Page *page)
{
    page->frames = g_list_first (page->frames);
    if (page->frames != NULL)
        return page->frames->data;
    return NULL;
}

GList *
tbo_page_get_frames (Page *page)
{
    return g_list_first (page->frames);
}

void
tbo_page_save (Page *page, FILE *file)
{
    char buffer[255];
    GList *f;
    snprintf (buffer, 255, " <page>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    for (f=g_list_first (page->frames); f; f = g_list_next(f))
    {
        tbo_frame_save ((Frame *) f->data, file);
    }

    snprintf (buffer, 255, " </page>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
}
