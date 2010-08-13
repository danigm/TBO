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


#ifndef __TBO_COMIC__
#define __TBO_COMIC__

#include <gtk/gtk.h>
#include "tbo-types.h"
#include "tbo-window.h"

Comic *tbo_comic_new (const char *title, int width, int height);
void tbo_comic_free (Comic *comic);
Page *tbo_comic_new_page (Comic *comic);
void tbo_comic_del_page (Comic *comic, int nth);
gboolean tbo_comic_del_current_page (Comic *comic);
int tbo_comic_len (Comic *comic);
int tbo_comic_page_index (Comic *comic);
int tbo_comic_page_nth (Comic *comic, Page *page);
gboolean tbo_comic_page_first (Comic *comic);
gboolean tbo_comic_page_last (Comic *comic);
Page *tbo_comic_next_page (Comic *comic);
Page *tbo_comic_prev_page (Comic *comic);
Page *tbo_comic_get_current_page (Comic *comic);
void tbo_comic_set_current_page (Comic *comic, Page *page);
void tbo_comic_set_current_page_nth (Comic *comic, int nth);
void tbo_comic_save (TboWindow *tbo, char *filename);
void tbo_comic_open (TboWindow *window, char *filename);

#endif

