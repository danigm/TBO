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


#ifndef __TBO_TOOLBAR_H__
#define __TBO_TOOLBAR_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include "tbo-tool-base.h"
#include "tbo-types.h"

#define TBO_TYPE_TOOLBAR            (tbo_toolbar_get_type ())
#define TBO_TOOLBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_TOOLBAR, TboToolbar))
#define TBO_IS_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_TOOLBAR))
#define TBO_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_TOOLBAR, TboToolbarClass))
#define TBO_IS_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_TOOLBAR))
#define TBO_TOOLBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_TOOLBAR, TboToolbarClass))

typedef struct _TboToolbar      TboToolbar;
typedef struct _TboToolbarClass TboToolbarClass;

struct _TboToolbar
{
    GObject parent_instance;

    /* instance members */
    TboWindow *tbo;

    TboToolBase *selected_tool;
    GtkActionGroup *action_group;
    GtkWidget *toolbar;
    TboToolBase **tools;
};

struct _TboToolbarClass
{
    GObjectClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_TOOLBAR */
GType tbo_toolbar_get_type (void);

/*
 * Method definitions.
 */

enum Tool
{
    TBO_TOOLBAR_NONE,
    TBO_TOOLBAR_SELECTOR,
    TBO_TOOLBAR_FRAME,
    TBO_TOOLBAR_DOODLE,
    TBO_TOOLBAR_BUBBLE,
    TBO_TOOLBAR_TEXT,
    TBO_TOOLBAR_N_TOOLS
};

GObject * tbo_toolbar_new ();
GObject * tbo_toolbar_new_with_params (TboWindow *tbo);

TboToolBase * tbo_toolbar_get_selected_tool (TboToolbar *self);
void tbo_toolbar_set_selected_tool (TboToolbar *self, enum Tool tool);
GtkWidget * tbo_toolbar_get_toolbar (TboToolbar *self);
void tbo_toolbar_update (TboToolbar *self);

#endif /* __TBO_TOOLBAR_H__ */

