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


#ifndef __TBO_TOOL_BASE_H__
#define __TBO_TOOL_BASE_H__

#include <glib-object.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include "tbo-types.h"

#define TBO_TYPE_TOOL_BASE            (tbo_tool_base_get_type ())
#define TBO_TOOL_BASE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_TOOL_BASE, TboToolBase))
#define TBO_IS_TOOL_BASE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_TOOL_BASE))
#define TBO_TOOL_BASE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_TOOL_BASE, TboToolBaseClass))
#define TBO_IS_TOOL_BASE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_TOOL_BASE))
#define TBO_TOOL_BASE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_TOOL_BASE, TboToolBaseClass))

typedef struct _TboToolBase      TboToolBase;
typedef struct _TboToolBaseClass TboToolBaseClass;

struct _TboToolBase
{
    GObject parent_instance;

    /* instance members */
    TboWindow *tbo;
    gchar *action;

    void (*on_select) (TboToolBase *tool);
    void (*on_unselect) (TboToolBase *tool);
    void (*on_move) (TboToolBase *tool, GtkWidget *widget, GdkEventMotion *event);
    void (*on_click) (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event);
    void (*on_release) (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event);
    void (*on_key) (TboToolBase *tool, GtkWidget *widget, GdkEventKey *event);
    void (*drawing) (TboToolBase *tool, cairo_t *cr);
};

struct _TboToolBaseClass
{
    GObjectClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_TOOL_BASE */
GType tbo_tool_base_get_type (void);

/*
 * Method definitions.
 */
GObject * tbo_tool_base_new ();
GObject * tbo_tool_base_new_with_params (TboWindow *tbo);
void tbo_tool_base_set_action (TboToolBase *self, gchar *action);

#endif /* __TBO_TOOL_BASE_H__ */

