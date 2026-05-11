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


#ifndef __TBO_EXPORT__
#define __TBO_EXPORT__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-window.h"

typedef enum
{
    TBO_EXPORT_SCOPE_ALL_PAGES,
    TBO_EXPORT_SCOPE_CURRENT_PAGE,
    TBO_EXPORT_SCOPE_SELECTION,
} TboExportScope;

gboolean tbo_export (TboWindow *tbo);
gboolean tbo_export_file (TboWindow *tbo,
                          const gchar *filename,
                          const gchar *format_hint,
                          gint width,
                          gint height);
gboolean tbo_export_file_with_scope (TboWindow *tbo,
                                     const gchar *filename,
                                     const gchar *format_hint,
                                     gint width,
                                     gint height,
                                     TboExportScope scope);
gboolean tbo_export_file_with_scope_range (TboWindow *tbo,
                                           const gchar *filename,
                                           const gchar *format_hint,
                                           gint width,
                                           gint height,
                                           TboExportScope scope,
                                           gint from_page,
                                           gint to_page);

#endif
