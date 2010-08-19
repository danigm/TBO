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


#ifndef __TBO_WINDOW__
#define __TBO_WINDOW__

#include <gtk/gtk.h>
#include "tbo-toolbar.h"
#include "tbo-types.h"

struct _TboWindow
{
    GtkWidget *window;
    GtkWidget *dw_scroll;
    GtkWidget *scroll2;
    GtkWidget *toolarea;
    GtkWidget *notebook;
    GtkWidget *drawing;
    GtkWidget *status;
    GtkWidget *vbox;
    TboToolbar *toolbar;
    Comic *comic;
    char *path;
};

TboWindow *tbo_window_new (GtkWidget *window, GtkWidget *dw_scroll, GtkWidget *scroll2, GtkWidget *notebook, GtkWidget *toolarea, GtkWidget *status, GtkWidget *vbox, Comic *comic);
void tbo_window_free (TboWindow *tbo);
gboolean tbo_window_free_cb (GtkWidget *widget, GdkEventExpose *event, TboWindow *tbo);
GdkPixbuf *create_pixbuf (const gchar * filename);
TboWindow * tbo_new_tbo (int width, int height);
void tbo_window_update_status (TboWindow *tbo, int x, int y);
void tbo_empty_tool_area (TboWindow *tbo);
void tbo_window_set_path (TboWindow *tbo, const char *path);
void tbo_window_set_current_tab_page (TboWindow *tbo, gboolean setit);
GtkWidget *create_darea (TboWindow *tbo);

#endif
