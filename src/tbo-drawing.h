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


#ifndef __TBO_DRAWING_H__
#define __TBO_DRAWING_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-types.h"
#include "tbo-tool-base.h"

#define TBO_TYPE_DRAWING            (tbo_drawing_get_type ())
#define TBO_DRAWING(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_DRAWING, TboDrawing))
#define TBO_IS_DRAWING(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_DRAWING))
#define TBO_DRAWING_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_DRAWING, TboDrawingClass))
#define TBO_IS_DRAWING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_DRAWING))
#define TBO_DRAWING_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_DRAWING, TboDrawingClass))

#define ZOOM_STEP 0.05

typedef struct _TboDrawing      TboDrawing;
typedef struct _TboDrawingClass TboDrawingClass;

struct _TboDrawing
{
    GtkLayout parent_instance;

    /* instance members */
    TboToolBase *tool;
    Frame *current_frame;
    gdouble zoom;
    Comic *comic;
};

struct _TboDrawingClass
{
    GtkLayoutClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_DRAWING */
GType tbo_drawing_get_type (void);

/*
 * Method definitions.
 */

GtkWidget * tbo_drawing_new ();
GtkWidget * tbo_drawing_new_with_params (Comic *comic);
void tbo_drawing_update (TboDrawing *self);
void tbo_drawing_set_current_frame (TboDrawing *self, Frame *frame);
Frame * tbo_drawing_get_current_frame (TboDrawing *self);
void tbo_drawing_draw (TboDrawing *self, cairo_t *cr);
void tbo_drawing_draw_page (TboDrawing *self, cairo_t *cr, Page *page, gint w, gint h);
void tbo_drawing_zoom_in (TboDrawing *self);
void tbo_drawing_zoom_out (TboDrawing *self);
void tbo_drawing_zoom_100 (TboDrawing *self);
void tbo_drawing_zoom_fit (TboDrawing *self);
gdouble tbo_drawing_get_zoom (TboDrawing *self);
void tbo_drawing_adjust_scroll (TboDrawing *self);
void tbo_drawing_init_dnd (TboDrawing *self, TboWindow *tbo);

#endif /* __TBO_DRAWING_H__ */

