/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2011  Daniel Garcia Moreno <danigm@wadobo.com>
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

#ifndef _TBO_STARTUP_H
#define _TBO_STARTUP_H

#include "gtk/gtk.h"
#include "glib-object.h"

#define TBO_TYPE_STARTUP            (tbo_startup_get_type ())
#define TBO_STARTUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_STARTUP, TboStartup))
#define TBO_IS_STARTUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_STARTUP))
#define TBO_STARTUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_STARTUP, TboStartupClass))
#define TBO_IS_STARTUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_STARTUP))
#define TBO_STARTUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_STARTUP, TboStartupClass))

typedef struct _TboStartup    TboStartup;
typedef struct _TboStartupClass TboStartupClass;

struct _TboStartup {
    GtkGrid parent;

    /* instance members */
};

struct _TboStartupClass {
    GtkGridClass parent_class;

    /* class members */
};

GType tbo_startup_get_type (void);

/*
 * Method definitions.
 */

GtkWidget * tbo_startup_new ();

#endif
