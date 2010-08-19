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


#ifndef __TBO_TOOL_TEXT_H__
#define __TBO_TOOL_TEXT_H__

#include <glib-object.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include "tbo-tool-base.h"
#include "tbo-window.h"
#include "tbo-object-text.h"

#define TBO_TYPE_TOOL_TEXT            (tbo_tool_text_get_type ())
#define TBO_TOOL_TEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_TOOL_TEXT, TboToolText))
#define TBO_IS_TOOL_TEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_TOOL_TEXT))
#define TBO_TOOL_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_TOOL_TEXT, TboToolTextClass))
#define TBO_IS_TOOL_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_TOOL_TEXT))
#define TBO_TOOL_TEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_TOOL_TEXT, TboToolTextClass))

typedef struct _TboToolText      TboToolText;
typedef struct _TboToolTextClass TboToolTextClass;

struct _TboToolText
{
    TboToolBase parent_instance;

    /* instance members */
    GtkWidget *font;
    GtkWidget *font_color;
    TboObjectText *text_selected;
    GtkTextBuffer *text_buffer;
};

struct _TboToolTextClass
{
    TboToolBaseClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_TOOL_TEXT */
GType tbo_tool_text_get_type (void);

/*
 * Method definitions.
 */
GObject * tbo_tool_text_new ();
GObject * tbo_tool_text_new_with_params (TboWindow *tbo);
gchar * tbo_tool_text_get_pango_font (TboToolText *self);
gchar * tbo_tool_text_get_font_name (TboToolText *self);
void tbo_tool_text_set_selected (TboToolText *self, TboObjectText *text);

#endif /* __TBO_TOOL_TEXT_H__ */

