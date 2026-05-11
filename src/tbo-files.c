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


#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "tbo-files.h"
#include <glib.h>
#include "tbo-utils.h"

char **tbo_files_get_dirs (void)
{
    char **possible_dirs = g_new0 (char *, 4);

    possible_dirs[0] = g_build_filename (g_get_home_dir (), ".tbo", "doodle", NULL);
    possible_dirs[1] = g_build_filename (g_get_user_data_dir (), "tbo", "doodle", NULL);
    possible_dirs[2] = tbo_get_data_path ("doodle");

    return possible_dirs;
}

int
tbo_files_prefix_len (char *str)
{
    int n = 0, i = 0;
    char **possible_dirs = tbo_files_get_dirs ();
    while (possible_dirs[i])
    {
        if (g_str_has_prefix (str, possible_dirs[i]))
        {
            n = strlen (possible_dirs[i]) + 1;
            break;
        }
        i++;
    }
    tbo_files_free (possible_dirs);
    return n;
}

void
tbo_files_free (char **files)
{
    int i = 0;
    while(files[i])
    {
        g_free (files[i]);
        i++;
    }
    g_free (files);
}

gchar *
tbo_files_expand_path (const gchar *source)
{
    int st, i = 0;
    char **possible_dirs = tbo_files_get_dirs ();
    struct stat filestat;
    gchar *dest = NULL;

    while (possible_dirs[i])
    {
        g_free (dest);
        dest = g_build_filename (possible_dirs[i], source, NULL);
        st = stat (dest, &filestat);
        if (!st)
            break;

        i++;
    }

    if (dest == NULL || stat (dest, &filestat) != 0)
    {
        g_free (dest);
        dest = g_strdup (source);
    }

    tbo_files_free (possible_dirs);
    return dest;
}

gboolean
tbo_files_is_svg_file (const gchar *source)
{
    const gchar *ext;

    if (source == NULL || *source == '\0')
        return FALSE;

    ext = strrchr (source, '.');
    if (ext == NULL)
        return FALSE;

    return g_ascii_strcasecmp (ext + 1, "svg") == 0;
}

gboolean
tbo_files_is_supported_asset_file (const gchar *source)
{
    GdkPixbufFormat *format;
    gchar *path;
    gboolean is_supported = FALSE;

    if (tbo_files_is_svg_file (source))
        return TRUE;

    if (source == NULL || *source == '\0')
        return FALSE;

    path = tbo_files_expand_path (source);
    format = gdk_pixbuf_get_file_info (path, NULL, NULL);
    if (format != NULL)
        is_supported = TRUE;

    g_free (path);

    return is_supported;
}
