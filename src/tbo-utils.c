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


#include <locale.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <errno.h>
#include "config.h"
#include "tbo-object-base.h"
#include "tbo-utils.h"


void
get_base_name (const gchar *str, gchar *ret, int size)
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

gchar *
tbo_get_data_path (const gchar *relative_path)
{
    gchar *installed_path = g_build_filename (DATA_DIR, relative_path, NULL);

    if (g_file_test (installed_path, G_FILE_TEST_EXISTS))
        return installed_path;

    g_free (installed_path);
    return g_build_filename (SOURCE_DATA_DIR, relative_path, NULL);
}

gchar *
tbo_get_locale_path (void)
{
    gchar *exe_path;
    gchar *exe_dir;
    gchar *build_locale_dir;

    exe_path = g_file_read_link ("/proc/self/exe", NULL);
    if (exe_path == NULL)
        return g_strdup (GNOMELOCALEDIR);

    exe_dir = g_path_get_dirname (exe_path);
    build_locale_dir = g_build_filename (exe_dir, "po", NULL);

    g_free (exe_path);
    g_free (exe_dir);

    if (g_file_test (build_locale_dir, G_FILE_TEST_IS_DIR))
        return build_locale_dir;

    g_free (build_locale_dir);
    return g_strdup (GNOMELOCALEDIR);
}

void
tbo_init_i18n (void)
{
    gchar *locale_dir;

    setlocale (LC_ALL, "");

#ifdef ENABLE_NLS
    locale_dir = tbo_get_locale_path ();
    bindtextdomain (GETTEXT_PACKAGE, locale_dir);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
    g_free (locale_dir);
#endif
}

const gchar *
tbo_ascii_formatd (gchar *buffer, gsize buffer_len, gdouble value)
{
    return g_ascii_formatd (buffer, buffer_len, "%.17g", value);
}

gboolean
tbo_ascii_parse_int (const gchar *text, gint *value)
{
    gchar *endptr = NULL;
    gint64 parsed;

    if (text == NULL || *text == '\0')
        return FALSE;

    errno = 0;
    parsed = g_ascii_strtoll (text, &endptr, 10);
    if (endptr == text || *endptr != '\0' || errno == ERANGE ||
        parsed < G_MININT || parsed > G_MAXINT)
        return FALSE;

    if (value != NULL)
        *value = (gint) parsed;

    return TRUE;
}

gboolean
tbo_ascii_parse_double (const gchar *text, gdouble *value)
{
    gchar *normalized = NULL;
    gchar *endptr = NULL;
    gdouble parsed;

    if (text == NULL || *text == '\0')
        return FALSE;

    errno = 0;
    parsed = g_ascii_strtod (text, &endptr);
    if (endptr == text || *endptr != '\0' || errno == ERANGE)
    {
        if (strchr (text, ',') == NULL || strchr (text, '.') != NULL)
            return FALSE;

        normalized = g_strdup (text);
        g_strdelimit (normalized, ",", '.');
        endptr = NULL;
        errno = 0;
        parsed = g_ascii_strtod (normalized, &endptr);
        if (endptr == normalized || *endptr != '\0' || errno == ERANGE)
        {
            g_free (normalized);
            return FALSE;
        }
        g_free (normalized);
    }

    if (value != NULL)
        *value = parsed;

    return TRUE;
}

void
tbo_xml_append_attr_int (GString *xml, const gchar *name, gint value)
{
    g_string_append_printf (xml, " %s=\"%d\"", name, value);
}

void
tbo_xml_append_attr_double (GString *xml, const gchar *name, gdouble value)
{
    gchar buffer[G_ASCII_DTOSTR_BUF_SIZE];

    tbo_ascii_formatd (buffer, sizeof (buffer), value);
    g_string_append_printf (xml, " %s=\"%s\"", name, buffer);
}

void
tbo_xml_append_attr_string (GString *xml, const gchar *name, const gchar *value)
{
    gchar *escaped = g_markup_escape_text (value != NULL ? value : "", -1);

    g_string_append_printf (xml, " %s=\"%s\"", name, escaped);
    g_free (escaped);
}

void
tbo_xml_append_object_attrs (GString *xml, TboObjectBase *self)
{
    tbo_xml_append_attr_int (xml, "x", self->x);
    tbo_xml_append_attr_int (xml, "y", self->y);
    tbo_xml_append_attr_int (xml, "width", self->width);
    tbo_xml_append_attr_int (xml, "height", self->height);
    tbo_xml_append_attr_double (xml, "angle", self->angle);
    tbo_xml_append_attr_int (xml, "flipv", self->flipv);
    tbo_xml_append_attr_int (xml, "fliph", self->fliph);
}

void
tbo_xml_write (FILE *file, GString *xml)
{
    fputs (xml->str, file);
    g_string_free (xml, TRUE);
}
