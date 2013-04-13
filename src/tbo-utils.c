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


#include <gtk/gtk.h>
#include "tbo-utils.h"


void
get_base_name (gchar *str, gchar *ret, int size)
{
    gchar **paths;
    gchar **dirname;
    paths = g_strsplit (str, "/", 0);
    dirname = paths;
    while (*dirname) dirname++;
    dirname--;
    snprintf (ret, size, "%s", *dirname);
    g_strfreev (paths);
}
