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

#include <glib-object.h>
#include <gtk/gtk.h>
#include "tbo-types.h"
#include "tbo-window.h"

#define TBO_TYPE_COMIC            (tbo_comic_get_type ())
#define TBO_COMIC(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_COMIC, Comic))
#define TBO_IS_COMIC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_COMIC))
#define TBO_COMIC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_COMIC, ComicClass))
#define TBO_IS_COMIC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_COMIC))
#define TBO_COMIC_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_COMIC, ComicClass))

typedef struct _ComicClass ComicClass;

typedef enum
{
    TBO_COMIC_PAPER_NONE,
    TBO_COMIC_PAPER_A4,
} TboComicPaper;

GType tbo_comic_get_type (void);

Comic *tbo_comic_new (const char *title, int width, int height);
void tbo_comic_free (Comic *comic);
const gchar *tbo_comic_get_title (Comic *comic);
gint tbo_comic_get_width (Comic *comic);
gint tbo_comic_get_height (Comic *comic);
TboComicPaper tbo_comic_get_paper (Comic *comic);
void tbo_comic_set_paper (Comic *comic, TboComicPaper paper);
gboolean tbo_comic_get_pdf_page_size (Comic *comic, gdouble *width, gdouble *height);
GList *tbo_comic_get_pages (Comic *comic);
Page *tbo_comic_new_page (Comic *comic);
void tbo_comic_insert_page (Comic *comic, Page *page, int nth);
void tbo_comic_del_page (Comic *comic, int nth);
gboolean tbo_comic_del_current_page (Comic *comic);
int tbo_comic_len (Comic *comic);
int tbo_comic_page_index (Comic *comic);
int tbo_comic_page_position (Comic *comic);
int tbo_comic_page_nth (Comic *comic, Page *page);
gboolean tbo_comic_page_first (Comic *comic);
gboolean tbo_comic_page_last (Comic *comic);
Page *tbo_comic_next_page (Comic *comic);
Page *tbo_comic_prev_page (Comic *comic);
Page *tbo_comic_get_current_page (Comic *comic);
void tbo_comic_set_current_page (Comic *comic, Page *page);
void tbo_comic_set_current_page_nth (Comic *comic, int nth);
void tbo_comic_reorder_page (Comic *comic, Page *page, int nth);
gboolean tbo_comic_save (TboWindow *tbo, const gchar *filename);
gboolean tbo_comic_save_snapshot (TboWindow *tbo, const gchar *filename);
gboolean tbo_comic_open (TboWindow *window, const gchar *filename);

#endif
