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


#ifndef __TBO_PAGE__
#define __TBO_PAGE__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include "tbo-types.h"

#define TBO_TYPE_PAGE            (tbo_page_get_type ())
#define TBO_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_PAGE, Page))
#define TBO_IS_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_PAGE))
#define TBO_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_PAGE, PageClass))
#define TBO_IS_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_PAGE))
#define TBO_PAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_PAGE, PageClass))

typedef struct _PageClass PageClass;

GType tbo_page_get_type (void);

Page *tbo_page_new (Comic *comic);
void tbo_page_free (Page *page);
Page *tbo_page_clone (Page *page);
Frame *tbo_page_new_frame (Page *page, int x, int y, int w, int h);
void tbo_page_add_frame (Page *page, Frame *frame);
void tbo_page_insert_frame (Page *page, Frame *frame, int nth);
void tbo_page_del_frame_by_index (Page *page, int nth);
void tbo_page_del_frame (Page *page, Frame *frame);
int tbo_page_len (Page *page);
int tbo_page_frame_index (Page *page);
int tbo_page_frame_position (Page *page);
int tbo_page_frame_nth (Page *page, Frame *frame);
gboolean tbo_page_frame_first (Page *page);
gboolean tbo_page_frame_last (Page *page);
Frame *tbo_page_first_frame (Page *page);
Frame *tbo_page_next_frame (Page *page);
Frame *tbo_page_prev_frame (Page *page);
Frame *tbo_page_get_current_frame (Page *page);
void tbo_page_set_current_frame (Page *page, Frame *frame);
GList *tbo_page_get_frames (Page *page);
void tbo_page_save (Page *page, FILE *file);

#endif
