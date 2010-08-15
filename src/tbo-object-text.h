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


#ifndef __TBO_OBJECT_TEXT_H__
#define __TBO_OBJECT_TEXT_H__

#include <glib.h>
#include <gtk/gtk.h>
#include "tbo-object-base.h"

#define TBO_TYPE_OBJECT_TEXT            (tbo_object_text_get_type ())
#define TBO_OBJECT_TEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_OBJECT_TEXT, TboObjectText))
#define TBO_IS_OBJECT_TEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_OBJECT_TEXT))
#define TBO_OBJECT_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_OBJECT_TEXT, TboObjectTextClass))
#define TBO_IS_OBJECT_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_OBJECT_TEXT))
#define TBO_OBJECT_TEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_OBJECT_TEXT, TboObjectTextClass))

#define COLORMAX 65535

typedef struct _TboObjectText      TboObjectText;
typedef struct _TboObjectTextClass TboObjectTextClass;

struct _TboObjectText
{
    TboObjectBase parent_instance;

    /* instance members */
    GString *text;
    PangoFontDescription *description;
    GdkColor *font_color;
};

struct _TboObjectTextClass
{
    TboObjectBaseClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_OBJECT_TEXT */
GType tbo_object_text_get_type (void);

/*
 * Method definitions.
 */

GObject * tbo_object_text_new ();
GObject * tbo_object_text_new_with_params (gint     x,
                                           gint     y,
                                           gint     width,
                                           gint     height,
                                           gchar    *text,
                                           gchar    *fontname,
                                           GdkColor *color);
gchar * tbo_object_text_get_text (TboObjectText *self);
void tbo_object_text_set_text (TboObjectText *self, const gchar *text);
void tbo_object_text_change_font (TboObjectText *self, gchar *font);
void tbo_object_text_change_color (TboObjectText *self, GdkColor *color);
gchar * tbo_object_text_get_string (TboObjectText *self);

#endif /* __TBO_OBJECT_TEXT_H__ */

