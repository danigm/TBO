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


#ifndef __TBO_TOOLTIP_H__
#define __TBO_TOOLTIP_H__

#include <cairo.h>
#include <glib.h>
#include "tbo-window.h"

void tbo_tooltip_set (const char *tooltip, int x, int y, TboWindow *tbo);
void tbo_tooltip_set_center_timeout (const char *tooltip, int timeout, TboWindow *tbo);
GString *tbo_tooltip_get ();
void tbo_tooltip_draw (cairo_t *cr);

#endif
