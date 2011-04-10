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


#include <string.h>
#include <gtk/gtk.h>
#include "dnd.h"
#include "tbo-drawing.h"
#include "frame.h"
#include "tbo-object-svg.h"
#include "tbo-window.h"

static GtkWidget *DND_IMAGE = NULL;

void
drag_data_received_handl (GtkWidget *widget,
                          GdkDragContext *context,
                          gint x, gint y,
                          GtkSelectionData *selection_data,
                          guint target_type,
                          guint time,
                          TboWindow *tbo)
{
    GtkAdjustment *adj;
    float zoom = tbo_drawing_get_zoom (TBO_DRAWING (tbo->drawing));
    const gchar   *_sdata;

    gboolean dnd_success = FALSE;
    gboolean delete_selection_data = FALSE;

    /* Deal with what we are given from source */
    if ((selection_data != NULL) && (gtk_selection_data_get_length (selection_data) >= 0))
    {
        if (gdk_drag_context_get_selected_action (context) == GDK_ACTION_ASK)
        {
            /* Ask the user to move or copy, then set the context action. */
        }

        if (gdk_drag_context_get_selected_action (context) == GDK_ACTION_MOVE)
            delete_selection_data = TRUE;

        /* Check that we got the format we can use */
        switch (target_type)
        {
            case TARGET_STRING:
                _sdata = gtk_selection_data_get_data (selection_data);

                Frame *frame = tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing));
                adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (tbo->dw_scroll));
                int rx = tbo_frame_get_base_x ((x + gtk_adjustment_get_value(adj)) / zoom);
                adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (tbo->dw_scroll));
                int ry = tbo_frame_get_base_y ((y + gtk_adjustment_get_value(adj)) / zoom);

                TboObjectSvg *svgimage = TBO_OBJECT_SVG (tbo_object_svg_new_with_params (rx, ry, 0, 0, (gchar*)_sdata));
                tbo_drawing_update (TBO_DRAWING (tbo->drawing));
                tbo_frame_add_obj (frame, TBO_OBJECT_BASE (svgimage));

                dnd_success = TRUE;
                break;

            default:
                g_print ("nothing good");
        }
    }

    if (dnd_success == FALSE)
    {
        g_print ("DnD data transfer failed!\n");
    }

    gtk_drag_finish (context, dnd_success, delete_selection_data, time);
}

void
drag_data_get_handl (GtkWidget *widget,
                     GdkDragContext *context,
                     GtkSelectionData *selection_data,
                     guint target_type,
                     guint time,
                     char *svg)
{
    g_assert (selection_data != NULL);
    switch (target_type)
    {
        case TARGET_STRING:
            gtk_selection_data_set (selection_data,
                                    gtk_selection_data_get_target (selection_data),
                                    8*sizeof(char),
                                    (guchar*) svg,
                                    strlen (svg));
            break;
        default:
            /* Default to some a safe target instead of fail. */
            g_assert_not_reached ();
    }
}

void
drag_begin_handl (GtkWidget *widget,
                  GdkDragContext *context,
                  char *svg)
{
    DND_IMAGE = gtk_image_new_from_file (svg);
    GdkPixbuf *pix = gtk_image_get_pixbuf (GTK_IMAGE (DND_IMAGE));
    gtk_drag_set_icon_pixbuf (context, pix, 0, 0);
}

void
drag_end_handl (GtkWidget *widget,
                GdkDragContext *context,
                gpointer user_data)
{
    if (DND_IMAGE)
    {
        gtk_widget_destroy (GTK_WIDGET (DND_IMAGE));
        DND_IMAGE = NULL;
    }
}
