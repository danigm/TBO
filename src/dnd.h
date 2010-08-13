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


#ifndef __TBO_DND__
#define __TBO_DND__

#include <gtk/gtk.h>
#include "tbo-window.h"

enum {
    TARGET_STRING,
};


static GtkTargetEntry TARGET_LIST[] = {
    { "STRING",     0, TARGET_STRING },
    { "text/plain", 0, TARGET_STRING },
};

static guint N_TARGETS = G_N_ELEMENTS (TARGET_LIST);

// destination signals
void drag_data_received_handl (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint target_type, guint time, TboWindow *tbo);

// source signals
void drag_data_get_handl (GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint target_type, guint time, char *svg);
void drag_begin_handl (GtkWidget *widget, GdkDragContext *context, char *svg);
void drag_end_handl (GtkWidget *widget, GdkDragContext *context, gpointer user_data);

#endif
