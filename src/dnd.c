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


#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "dnd.h"
#include "tbo-drawing.h"
#include "frame.h"
#include "tbo-object-svg.h"
#include "tbo-object-pixmap.h"
#include "tbo-tooltip.h"
#include "tbo-files.h"
#include "tbo-window.h"
#include "tbo-tool-selector.h"
#include "tbo-undo.h"
#include "tbo-widget.h"

#define TBO_DND_MAX_FRAME_WIDTH_FRACTION 1.0
#define TBO_DND_MAX_FRAME_HEIGHT_FRACTION 0.5

typedef enum
{
    TBO_DND_INSERT_OK,
    TBO_DND_INSERT_NO_FRAME,
    TBO_DND_INSERT_OUTSIDE_FRAME,
    TBO_DND_INSERT_INVALID_ASSET,
} TboDndInsertResult;

#define TBO_DND_FEEDBACK_TIMEOUT_MS 2500

static TboObjectBase *
create_asset (const gchar *asset_path, gint x, gint y)
{
    if (tbo_files_is_svg_file (asset_path))
        return TBO_OBJECT_BASE (tbo_object_svg_new_with_params (x, y, 0, 0, (gchar *) asset_path));

    if (!tbo_files_is_supported_asset_file (asset_path))
        return NULL;

    return TBO_OBJECT_BASE (tbo_object_pixmap_new_with_params (x, y, 0, 0, (gchar *) asset_path));
}

static gboolean
get_svg_asset_size (const gchar *asset_path, gint *width, gint *height)
{
    GError *error = NULL;
    RsvgHandle *handle;
    gdouble width_px = 0.0;
    gdouble height_px = 0.0;
    gchar *path;
    gboolean ok = FALSE;

    path = tbo_files_expand_path (asset_path);
    handle = rsvg_handle_new_from_file (path, &error);
    if (handle != NULL)
    {
        ok = rsvg_handle_get_intrinsic_size_in_pixels (handle, &width_px, &height_px) &&
             width_px > 0.0 && height_px > 0.0;
        g_object_unref (handle);
    }
    if (error != NULL)
        g_error_free (error);
    g_free (path);

    if (!ok)
        return FALSE;

    *width = MAX (1, (gint) ceil (width_px));
    *height = MAX (1, (gint) ceil (height_px));
    return TRUE;
}

static gboolean
get_pixbuf_asset_size (const gchar *asset_path, gint *width, gint *height)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    gchar *path;
    gboolean ok = FALSE;

    path = tbo_files_expand_path (asset_path);
    pixbuf = gdk_pixbuf_new_from_file (path, &error);
    if (pixbuf != NULL)
    {
        *width = gdk_pixbuf_get_width (pixbuf);
        *height = gdk_pixbuf_get_height (pixbuf);
        ok = *width > 0 && *height > 0;
        g_object_unref (pixbuf);
    }
    if (error != NULL)
        g_error_free (error);
    g_free (path);

    return ok;
}

static gboolean
get_asset_size (const gchar *asset_path, gint *width, gint *height)
{
    if (asset_path == NULL || width == NULL || height == NULL)
        return FALSE;

    if (tbo_files_is_svg_file (asset_path))
        return get_svg_asset_size (asset_path, width, height);

    return get_pixbuf_asset_size (asset_path, width, height);
}

static void
apply_initial_asset_size_limit (Frame *frame, TboObjectBase *asset, const gchar *asset_path)
{
    gint asset_width;
    gint asset_height;
    gint max_width;
    gint max_height;
    gdouble scale;

    if (frame == NULL || asset == NULL)
        return;
    if (!get_asset_size (asset_path, &asset_width, &asset_height))
        return;

    max_width = MAX (1, (gint) floor (tbo_frame_get_width (frame) * TBO_DND_MAX_FRAME_WIDTH_FRACTION));
    max_height = MAX (1, (gint) floor (tbo_frame_get_height (frame) * TBO_DND_MAX_FRAME_HEIGHT_FRACTION));
    if (asset_width <= max_width && asset_height <= max_height)
        return;

    scale = MIN (max_width / (gdouble) asset_width,
                 max_height / (gdouble) asset_height);
    asset->width = MAX (1, (gint) round (asset_width * scale));
    asset->height = MAX (1, (gint) round (asset_height * scale));

    asset->x = CLAMP (asset->x - (asset->width / 2),
                      0,
                      MAX (0, tbo_frame_get_width (frame) - asset->width));
    asset->y = CLAMP (asset->y - (asset->height / 2),
                      0,
                      MAX (0, tbo_frame_get_height (frame) - asset->height));
}

static void
select_inserted_asset (TboWindow *tbo, Frame *frame, TboObjectBase *asset);

static void
show_insert_feedback (TboWindow *tbo, TboDndInsertResult result)
{
    const gchar *message = NULL;

    switch (result)
    {
        case TBO_DND_INSERT_NO_FRAME:
            message = _("Enter a frame before inserting an image.");
            break;
        case TBO_DND_INSERT_OUTSIDE_FRAME:
            message = _("Drop the image inside the current frame.");
            break;
        case TBO_DND_INSERT_INVALID_ASSET:
            message = _("Couldn't insert the image.");
            break;
        case TBO_DND_INSERT_OK:
        default:
            break;
    }

    if (message != NULL)
        tbo_tooltip_set_center_timeout (message, TBO_DND_FEEDBACK_TIMEOUT_MS, tbo);
}

static TboDndInsertResult
insert_asset_into_frame (TboWindow *tbo,
                         const gchar *asset_path,
                         gint x,
                         gint y,
                         TboObjectBase **inserted_asset)
{
    Frame *frame = tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing));
    TboObjectBase *asset;

    if (inserted_asset != NULL)
        *inserted_asset = NULL;

    if (frame == NULL)
        return TBO_DND_INSERT_NO_FRAME;
    if (asset_path == NULL || *asset_path == '\0')
        return TBO_DND_INSERT_INVALID_ASSET;
    if (x < 0 || y < 0 || x > tbo_frame_get_width (frame) || y > tbo_frame_get_height (frame))
        return TBO_DND_INSERT_OUTSIDE_FRAME;

    asset = create_asset (asset_path, x, y);
    if (asset == NULL)
        return TBO_DND_INSERT_INVALID_ASSET;

    apply_initial_asset_size_limit (frame, asset, asset_path);

    tbo_frame_add_obj (frame, asset);
    tbo_undo_stack_insert (tbo->undo_stack, tbo_action_object_add_new (frame, asset));
    select_inserted_asset (tbo, frame, asset);
    tbo_window_mark_dirty (tbo);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    tbo_toolbar_update (tbo->toolbar);

    if (inserted_asset != NULL)
        *inserted_asset = asset;

    return TBO_DND_INSERT_OK;
}

static void
select_inserted_asset (TboWindow *tbo, Frame *frame, TboObjectBase *asset)
{
    TboToolSelector *selector;
    TboToolBase *current_tool = tbo->toolbar->selected_tool;

    if (current_tool == tbo->toolbar->tools[TBO_TOOLBAR_DOODLE] ||
        current_tool == tbo->toolbar->tools[TBO_TOOLBAR_BUBBLE])
    {
        return;
    }

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    tbo_tool_selector_set_selected (selector, frame);
    tbo_tool_selector_set_selected_obj (selector, asset);
}

static const gchar *
get_drag_asset_path (GtkWidget *widget, const gchar *key, gpointer fallback)
{
    const gchar *path = g_object_get_data (G_OBJECT (widget), key);

    if (path != NULL)
        return path;

    return fallback;
}

static GdkContentProvider *
drag_prepare_handl (GtkDragSource *source,
                    gdouble        x,
                    gdouble        y,
                    gpointer       user_data)
{
    GtkWidget *widget = GTK_WIDGET (user_data);
    const gchar *asset_path = get_drag_asset_path (widget, "tbo-asset-relative-path", NULL);

    if (asset_path == NULL)
        return NULL;

    return gdk_content_provider_new_typed (G_TYPE_STRING, asset_path);
}

static void
drag_begin_handl (GtkDragSource *source,
                  GdkDrag       *drag,
                  gpointer       user_data)
{
    GtkWidget *widget = GTK_WIDGET (user_data);
    GtkWidget *child = tbo_widget_get_first_child (widget);
    GdkPaintable *paintable = NULL;

    if (GTK_IS_PICTURE (child))
        paintable = gtk_picture_get_paintable (GTK_PICTURE (child));
    else if (GTK_IS_IMAGE (child))
        paintable = gtk_image_get_paintable (GTK_IMAGE (child));

    if (paintable != NULL)
        gtk_drag_source_set_icon (source, paintable, 0, 0);
}

static gboolean
drop_handl (GtkDropTarget *target,
            const GValue  *value,
            gdouble        x,
            gdouble        y,
            gpointer       user_data)
{
    TboWindow *tbo = user_data;
    GtkAdjustment *adj;
    gdouble zoom = tbo_drawing_get_zoom (TBO_DRAWING (tbo->drawing));
    const gchar *asset_path = g_value_get_string (value);

    if (asset_path == NULL)
        return FALSE;

    adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (tbo->dw_scroll));
    x = (x + gtk_adjustment_get_value (adj)) / zoom;
    adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (tbo->dw_scroll));
    y = (y + gtk_adjustment_get_value (adj)) / zoom;

    return tbo_dnd_insert_asset_at_view_coords (tbo, asset_path, x, y) != NULL;
}

TboObjectBase *
tbo_dnd_insert_asset_at_view_coords (TboWindow *tbo, const gchar *asset_path, gdouble x, gdouble y)
{
    gint frame_x;
    gint frame_y;
    TboObjectBase *asset = NULL;
    TboDndInsertResult result;

    if (!tbo_drawing_view_to_frame (TBO_DRAWING (tbo->drawing), x, y, &frame_x, &frame_y))
    {
        show_insert_feedback (tbo, TBO_DND_INSERT_NO_FRAME);
        return NULL;
    }

    result = insert_asset_into_frame (tbo, asset_path, frame_x, frame_y, &asset);
    if (result != TBO_DND_INSERT_OK)
    {
        show_insert_feedback (tbo, result);
        return NULL;
    }

    return asset;
}

TboObjectBase *
tbo_dnd_insert_asset (TboWindow *tbo, const gchar *asset_path, gint x, gint y)
{
    TboObjectBase *asset;
    TboDndInsertResult result;

    result = insert_asset_into_frame (tbo, asset_path, x, y, &asset);
    if (result != TBO_DND_INSERT_OK)
    {
        show_insert_feedback (tbo, result);
        return NULL;
    }

    return asset;
}

TboObjectBase *
tbo_dnd_insert_asset_centered (TboWindow *tbo, const gchar *asset_path)
{
    Frame *frame = tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing));

    if (frame == NULL)
    {
        show_insert_feedback (tbo, TBO_DND_INSERT_NO_FRAME);
        return NULL;
    }

    return tbo_dnd_insert_asset (tbo,
                                 asset_path,
                                 tbo_frame_get_width (frame) / 2,
                                 tbo_frame_get_height (frame) / 2);
}

void
tbo_dnd_setup_asset_source (GtkWidget *widget, const gchar *full_path, const gchar *relative_path)
{
    GtkDragSource *source = gtk_drag_source_new ();

    gtk_drag_source_set_actions (source, GDK_ACTION_COPY);
    g_object_set_data_full (G_OBJECT (widget), "tbo-asset-full-path", g_strdup (full_path), g_free);
    g_object_set_data_full (G_OBJECT (widget), "tbo-asset-relative-path", g_strdup (relative_path), g_free);
    g_signal_connect (source, "prepare", G_CALLBACK (drag_prepare_handl), widget);
    g_signal_connect (source, "drag-begin", G_CALLBACK (drag_begin_handl), widget);
    gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (source));
}

void
tbo_dnd_setup_drawing_dest (TboDrawing *drawing, TboWindow *tbo)
{
    GtkDropTarget *target = gtk_drop_target_new (G_TYPE_STRING, GDK_ACTION_COPY);

    g_signal_connect (target, "drop", G_CALLBACK (drop_handl), tbo);
    gtk_widget_add_controller (GTK_WIDGET (drawing), GTK_EVENT_CONTROLLER (target));
}
