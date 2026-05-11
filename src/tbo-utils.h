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


#ifndef __TBO_UTILS__
#define __TBO_UTILS__

#include <gtk/gtk.h>

typedef struct _TboObjectBase TboObjectBase;

void get_base_name (const gchar *str, gchar *ret, int size);
gchar *tbo_get_data_path (const gchar *relative_path);
gchar *tbo_get_locale_path (void);
void tbo_init_i18n (void);
const gchar *tbo_ascii_formatd (gchar *buffer, gsize buffer_len, gdouble value);
gboolean tbo_ascii_parse_int (const gchar *text, gint *value);
gboolean tbo_ascii_parse_double (const gchar *text, gdouble *value);
void tbo_xml_append_attr_int (GString *xml, const gchar *name, gint value);
void tbo_xml_append_attr_double (GString *xml, const gchar *name, gdouble value);
void tbo_xml_append_attr_string (GString *xml, const gchar *name, const gchar *value);
void tbo_xml_append_object_attrs (GString *xml, TboObjectBase *self);
void tbo_xml_write (FILE *file, GString *xml);

#endif
