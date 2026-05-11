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


#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>
#include <string.h>

#include "export.h"
#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-file-dialog.h"
#include "tbo-drawing.h"
#include "tbo-tool-selector.h"
#include "tbo-ui-utils.h"
#include "tbo-types.h"
#include "tbo-widget.h"

typedef struct
{
    GtkWidget *spinw;
    GtkWidget *spinh;
    gint base_width;
    gint base_height;
    gboolean updating;
} ExportSizeState;

typedef struct
{
    TboWindow *tbo;
    GtkEntry *entry;
} ExportFileArgs;

typedef struct
{
    TboWindow *tbo;
    ExportSizeState size_state;
    GtkWidget *scope_dropdown;
    GtkWidget *format_dropdown;
    GtkWidget *range_row;
    GtkWidget *range_from_spin;
    GtkWidget *range_to_spin;
    GtkWidget *preview_box;
    GtkWidget *preview_label;
    gint page_width;
    gint page_height;
    gint selection_width;
    gint selection_height;
    gboolean has_selection;
} ExportDialogState;

static TboExportScope dropdown_scope_to_export_scope (guint selected, gboolean has_selection);
static void draw_frame_export (cairo_t *cr, Frame *frame, gint width, gint height);

static gchar *
strip_matching_extension (const gchar *filename, const gchar *extension)
{
    const gchar *dot;

    if (filename == NULL || extension == NULL)
        return g_strdup (filename);

    dot = strrchr (filename, '.');
    if (dot != NULL && g_ascii_strcasecmp (dot + 1, extension) == 0)
        return g_strndup (filename, dot - filename);

    return g_strdup (filename);
}

static void
show_export_error (TboWindow *tbo, const gchar *message)
{
    tbo_alert_show (GTK_WINDOW (tbo->window), message, NULL);
}

static Frame *
get_export_selection_frame (TboWindow *tbo)
{
    TboDrawing *drawing;
    TboToolSelector *selector;
    Frame *current_frame;

    if (tbo == NULL || tbo->drawing == NULL || tbo->toolbar == NULL || tbo->toolbar->tools == NULL)
        return NULL;

    drawing = TBO_DRAWING (tbo->drawing);
    current_frame = tbo_drawing_get_current_frame (drawing);
    if (current_frame != NULL)
        return current_frame;

    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    return tbo_tool_selector_get_selected_frame (selector);
}

static gboolean
has_export_selection (TboWindow *tbo)
{
    return get_export_selection_frame (tbo) != NULL;
}

static void
get_export_scope_default_size (TboWindow *tbo, TboExportScope scope, gint *width, gint *height)
{
    Frame *selection;

    if (width == NULL || height == NULL)
        return;

    *width = tbo_comic_get_width (tbo->comic);
    *height = tbo_comic_get_height (tbo->comic);

    if (scope != TBO_EXPORT_SCOPE_SELECTION)
        return;

    selection = get_export_selection_frame (tbo);
    if (selection != NULL)
    {
        *width = tbo_frame_get_width (selection);
        *height = tbo_frame_get_height (selection);
    }
}

static void
normalize_export_page_range (Comic *comic, gint *from_page, gint *to_page)
{
    gint page_count;

    if (comic == NULL || from_page == NULL || to_page == NULL)
        return;

    page_count = MAX (1, tbo_comic_len (comic));
    *from_page = CLAMP (*from_page, 1, page_count);
    *to_page = CLAMP (*to_page, 1, page_count);

    if (*from_page > *to_page)
        *to_page = *from_page;
}

static GList *
build_export_page_range (Comic *comic, gint from_page, gint to_page, gint *n_pages)
{
    GList *pages = NULL;
    gint i;

    normalize_export_page_range (comic, &from_page, &to_page);
    for (i = from_page - 1; i <= to_page - 1; i++)
    {
        Page *page = g_list_nth_data (tbo_comic_get_pages (comic), i);

        if (page != NULL)
            pages = g_list_append (pages, page);
    }

    if (n_pages != NULL)
        *n_pages = g_list_length (pages);

    return pages;
}

static void
get_dialog_range (ExportDialogState *state, gint *from_page, gint *to_page)
{
    gint from_value;
    gint to_value;

    if (state == NULL)
        return;

    from_value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (state->range_from_spin));
    to_value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (state->range_to_spin));
    normalize_export_page_range (state->tbo->comic, &from_value, &to_value);

    if (from_page != NULL)
        *from_page = from_value;
    if (to_page != NULL)
        *to_page = to_value;
}

static Page *
get_preview_page_for_dialog (ExportDialogState *state, TboExportScope scope, gint from_page)
{
    if (state == NULL || state->tbo == NULL || state->tbo->comic == NULL)
        return NULL;

    if (scope == TBO_EXPORT_SCOPE_CURRENT_PAGE)
        return tbo_comic_get_current_page (state->tbo->comic);
    if (scope == TBO_EXPORT_SCOPE_ALL_PAGES)
        return g_list_nth_data (tbo_comic_get_pages (state->tbo->comic), MAX (0, from_page - 1));

    return NULL;
}

static GdkTexture *
create_texture_from_surface (cairo_surface_t *surface, gint width, gint height)
{
    GBytes *bytes;
    guchar *copy;
    gsize stride;
    gsize size;
    GdkTexture *texture;

    cairo_surface_flush (surface);
    stride = cairo_image_surface_get_stride (surface);
    size = stride * height;
    copy = g_memdup2 (cairo_image_surface_get_data (surface), size);
    bytes = g_bytes_new_take (copy, size);
    texture = gdk_memory_texture_new (width,
                                      height,
                                      GDK_MEMORY_DEFAULT,
                                      bytes,
                                      stride);
    g_bytes_unref (bytes);

    return texture;
}

static GdkTexture *
create_page_preview_texture (TboWindow *tbo, Page *page, gint width, gint height)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    GdkTexture *texture;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surface);
    tbo_drawing_draw_page (TBO_DRAWING (tbo->drawing), cr, page, width, height);
    texture = create_texture_from_surface (surface, width, height);
    cairo_destroy (cr);
    cairo_surface_destroy (surface);
    return texture;
}

static GdkTexture *
create_frame_preview_texture (Frame *frame, gint width, gint height)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    GdkTexture *texture;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surface);
    draw_frame_export (cr, frame, width, height);
    texture = create_texture_from_surface (surface, width, height);
    cairo_destroy (cr);
    cairo_surface_destroy (surface);
    return texture;
}

static void
set_preview_texture (ExportDialogState *state, GdkTexture *texture)
{
    GtkWidget *picture = gtk_picture_new_for_paintable (GDK_PAINTABLE (texture));

    gtk_picture_set_can_shrink (GTK_PICTURE (picture), TRUE);
    gtk_widget_set_size_request (picture, 220, 160);
    tbo_widget_destroy_all_children (state->preview_box);
    tbo_widget_add_child (state->preview_box, picture);
    tbo_widget_show_all (state->preview_box);
}

static void
update_preview_and_range (ExportDialogState *state)
{
    TboExportScope scope;
    gint from_page;
    gint to_page;
    gint width;
    gint height;
    gint preview_width;
    gint preview_height;
    GdkTexture *texture = NULL;
    gchar *label = NULL;
    gboolean range_sensitive;

    if (state == NULL)
        return;

    scope = dropdown_scope_to_export_scope (gtk_drop_down_get_selected (GTK_DROP_DOWN (state->scope_dropdown)),
                                            state->has_selection);
    get_dialog_range (state, &from_page, &to_page);

    range_sensitive = scope == TBO_EXPORT_SCOPE_ALL_PAGES && tbo_comic_len (state->tbo->comic) > 1;
    gtk_widget_set_sensitive (state->range_row, range_sensitive);

    width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (state->size_state.spinw));
    height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (state->size_state.spinh));
    preview_width = MAX (1, MIN (220, width));
    preview_height = MAX (1, MIN (160, (gint) ((preview_width * (gdouble) height) / MAX (1, width))));
    if (preview_height > 160)
    {
        preview_height = 160;
        preview_width = MAX (1, (gint) ((preview_height * (gdouble) width) / MAX (1, height)));
    }

    if (scope == TBO_EXPORT_SCOPE_SELECTION)
    {
        Frame *frame = get_export_selection_frame (state->tbo);

        if (frame != NULL)
            texture = create_frame_preview_texture (frame, preview_width, preview_height);
        label = g_strdup (_("Preview: Selection"));
    }
    else
    {
        Page *page = get_preview_page_for_dialog (state, scope, from_page);

        if (page != NULL)
            texture = create_page_preview_texture (state->tbo, page, preview_width, preview_height);

        if (scope == TBO_EXPORT_SCOPE_CURRENT_PAGE)
            label = g_strdup_printf (_("Preview: Current Page %d"), tbo_comic_page_position (state->tbo->comic));
        else if (from_page == to_page)
            label = g_strdup_printf (_("Preview: Page %d"), from_page);
        else
            label = g_strdup_printf (_("Preview: Page %d of Range %d-%d"), from_page, from_page, to_page);
    }

    gtk_label_set_text (GTK_LABEL (state->preview_label), label);
    set_preview_texture (state, texture);
    g_free (label);
    if (texture != NULL)
        g_object_unref (texture);
}

static void
set_export_size_base (ExportSizeState *state, gint width, gint height)
{
    if (state == NULL)
        return;

    state->base_width = MAX (1, width);
    state->base_height = MAX (1, height);

    state->updating = TRUE;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (state->spinw), state->base_width);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (state->spinh), state->base_height);
    state->updating = FALSE;
}

static gboolean
export_width_changed_cb (GtkWidget *widget, ExportSizeState *state)
{
    gint new_width;
    gint new_height;

    if (state == NULL || state->updating || state->base_width <= 0 || state->base_height <= 0)
        return FALSE;

    new_width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
    if (new_width <= 0)
        return FALSE;

    new_height = MAX (1, (gint) ((new_width * (gdouble) state->base_height) / state->base_width));
    state->updating = TRUE;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (state->spinh), new_height);
    state->updating = FALSE;
    return FALSE;
}

static gboolean
export_height_changed_cb (GtkWidget *widget, ExportSizeState *state)
{
    gint new_height;
    gint new_width;

    if (state == NULL || state->updating || state->base_width <= 0 || state->base_height <= 0)
        return FALSE;

    new_height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
    if (new_height <= 0)
        return FALSE;

    new_width = MAX (1, (gint) ((new_height * (gdouble) state->base_width) / state->base_height));
    state->updating = TRUE;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (state->spinw), new_width);
    state->updating = FALSE;
    return FALSE;
}

static gboolean
dialog_width_changed_cb (GtkWidget *widget, gpointer user_data)
{
    ExportDialogState *state = user_data;

    export_width_changed_cb (widget, &state->size_state);
    update_preview_and_range (state);
    return FALSE;
}

static gboolean
dialog_height_changed_cb (GtkWidget *widget, gpointer user_data)
{
    ExportDialogState *state = user_data;

    export_height_changed_cb (widget, &state->size_state);
    update_preview_and_range (state);
    return FALSE;
}

static TboExportScope
dropdown_scope_to_export_scope (guint selected, gboolean has_selection)
{
    if (selected == 0)
        return TBO_EXPORT_SCOPE_ALL_PAGES;
    if (selected == 1)
        return TBO_EXPORT_SCOPE_CURRENT_PAGE;
    if (has_selection && selected == 2)
        return TBO_EXPORT_SCOPE_SELECTION;

    return TBO_EXPORT_SCOPE_ALL_PAGES;
}

static void
scope_selected_cb (GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data)
{
    ExportDialogState *args = user_data;
    TboExportScope scope;

    (void) pspec;

    if (args == NULL)
        return;

    scope = dropdown_scope_to_export_scope (gtk_drop_down_get_selected (dropdown), args->has_selection);
    if (scope == TBO_EXPORT_SCOPE_SELECTION)
        set_export_size_base (&args->size_state, args->selection_width, args->selection_height);
    else
        set_export_size_base (&args->size_state, args->page_width, args->page_height);

    update_preview_and_range (args);
}

static gboolean
range_from_changed_cb (GtkWidget *widget, gpointer user_data)
{
    ExportDialogState *state = user_data;
    gint from_page = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
    gint to_page = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (state->range_to_spin));

    normalize_export_page_range (state->tbo->comic, &from_page, &to_page);
    if (to_page != gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (state->range_to_spin)))
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (state->range_to_spin), to_page);
    update_preview_and_range (state);
    return FALSE;
}

static gboolean
range_to_changed_cb (GtkWidget *widget, gpointer user_data)
{
    ExportDialogState *state = user_data;
    gint from_page = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (state->range_from_spin));
    gint to_page = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));

    normalize_export_page_range (state->tbo->comic, &from_page, &to_page);
    if (from_page != gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (state->range_from_spin)))
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (state->range_from_spin), from_page);
    update_preview_and_range (state);
    return FALSE;
}

static void
format_selected_cb (GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data)
{
    (void) dropdown;
    (void) pspec;
    update_preview_and_range (user_data);
}

static gboolean
filedialog_cb (GtkWidget *widget, gpointer data)
{
    ExportFileArgs *args = data;
    const gchar *current_text = gtk_editable_get_text (GTK_EDITABLE (args->entry));
    gchar *filename = tbo_file_dialog_save_export (args->tbo, current_text);

    (void) widget;

    if (filename != NULL)
    {
        gtk_editable_set_text (GTK_EDITABLE (args->entry), filename);
        tbo_window_set_export_path (args->tbo, filename);
        g_free (filename);
    }
    return FALSE;
}

static gboolean
begin_export_surface (TboWindow *tbo,
                      const gchar *export_to,
                      const gchar *path,
                      gint width,
                      gint height,
                      gboolean use_pdf_page_size,
                      cairo_surface_t **surface,
                      cairo_t **cr,
                      gdouble *draw_width,
                      gdouble *draw_height)
{
    if (g_strcmp0 (export_to, "pdf") == 0)
    {
        *draw_width = width;
        *draw_height = height;
        if (use_pdf_page_size && !tbo_comic_get_pdf_page_size (tbo->comic, draw_width, draw_height))
        {
            *draw_width = width;
            *draw_height = height;
        }
        *surface = cairo_pdf_surface_create (path, *draw_width, *draw_height);
    }
    else if (g_strcmp0 (export_to, "svg") == 0)
    {
        *draw_width = width;
        *draw_height = height;
        *surface = cairo_svg_surface_create (path, width, height);
    }
    else
    {
        *draw_width = width;
        *draw_height = height;
        *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    }

    *cr = cairo_create (*surface);
    if (cairo_surface_status (*surface) != CAIRO_STATUS_SUCCESS || cairo_status (*cr) != CAIRO_STATUS_SUCCESS)
    {
        show_export_error (tbo,
                           cairo_status_to_string (cairo_surface_status (*surface) != CAIRO_STATUS_SUCCESS ?
                                                   cairo_surface_status (*surface) :
                                                   cairo_status (*cr)));
        if (*surface != NULL)
            cairo_surface_destroy (*surface);
        if (*cr != NULL)
            cairo_destroy (*cr);
        *surface = NULL;
        *cr = NULL;
        return FALSE;
    }

    return TRUE;
}

static gboolean
finish_export_surface (TboWindow *tbo,
                       const gchar *export_to,
                       const gchar *path,
                       cairo_surface_t *surface,
                       cairo_t *cr)
{
    if (g_strcmp0 (export_to, "pdf") == 0)
        cairo_show_page (cr);
    else if (g_strcmp0 (export_to, "png") == 0)
    {
        cairo_status_t status = cairo_surface_write_to_png (surface, path);

        if (status != CAIRO_STATUS_SUCCESS)
        {
            show_export_error (tbo, cairo_status_to_string (status));
            return FALSE;
        }
    }

    return TRUE;
}

static void
draw_frame_export (cairo_t *cr, Frame *frame, gint width, gint height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);
    tbo_frame_draw_scaled (frame, cr, width, height);
}

static gboolean
export_page_list (TboWindow *tbo,
                  const gchar *base_filename,
                  const gchar *export_to,
                  gint width,
                  gint height,
                  GList *pages,
                  gint n_pages,
                  gboolean use_pdf_page_size)
{
    cairo_surface_t *surface = NULL;
    cairo_t *cr = NULL;
    gchar *format_pages = NULL;
    gboolean exported = FALSE;
    gboolean success = TRUE;
    gint digits = 0;
    gint count = n_pages;
    gint index = 0;

    if (pages == NULL || n_pages <= 0)
    {
        show_export_error (tbo, _("There are no pages to export."));
        return FALSE;
    }

    for (; count; count /= 10, digits++);
    format_pages = g_strdup_printf ("%%s%%0%dd.%%s", MAX (1, digits));

    for (; pages != NULL; pages = pages->next, index++)
    {
        Page *page = TBO_PAGE (pages->data);
        gchar *path = g_strdup_printf (format_pages, base_filename, index, export_to);
        gdouble draw_width;
        gdouble draw_height;

        if (page == NULL)
        {
            show_export_error (tbo, _("There are no pages to export."));
            g_free (path);
            success = FALSE;
            break;
        }

        if (n_pages == 1 || g_strcmp0 (export_to, "pdf") == 0)
        {
            g_free (path);
            path = g_strdup_printf ("%s.%s", base_filename, export_to);
        }

        if (g_strcmp0 (export_to, "pdf") == 0)
        {
            if (surface == NULL)
            {
                if (!begin_export_surface (tbo,
                                           export_to,
                                           path,
                                           width,
                                           height,
                                           use_pdf_page_size,
                                           &surface,
                                           &cr,
                                           &draw_width,
                                           &draw_height))
                {
                    g_free (path);
                    success = FALSE;
                    break;
                }
            }
            else
            {
                if (use_pdf_page_size && !tbo_comic_get_pdf_page_size (tbo->comic, &draw_width, &draw_height))
                {
                    draw_width = width;
                    draw_height = height;
                }
                else if (!use_pdf_page_size)
                {
                    draw_width = width;
                    draw_height = height;
                }
            }
        }
        else
        {
            if (!begin_export_surface (tbo,
                                       export_to,
                                       path,
                                       width,
                                       height,
                                       use_pdf_page_size,
                                       &surface,
                                       &cr,
                                       &draw_width,
                                       &draw_height))
            {
                g_free (path);
                success = FALSE;
                break;
            }
        }

        tbo_drawing_draw_page (TBO_DRAWING (tbo->drawing), cr, page, draw_width, draw_height);
        success = finish_export_surface (tbo, export_to, path, surface, cr);
        g_free (path);

        if (!success)
            break;

        exported = TRUE;

        if (g_strcmp0 (export_to, "pdf") != 0)
        {
            cairo_surface_destroy (surface);
            cairo_destroy (cr);
            surface = NULL;
            cr = NULL;
        }
    }

    if (surface != NULL)
    {
        cairo_surface_destroy (surface);
        cairo_destroy (cr);
    }

    g_free (format_pages);
    return success && exported;
}

static gboolean
export_single_frame (TboWindow *tbo,
                     const gchar *base_filename,
                     const gchar *export_to,
                     gint width,
                     gint height,
                     Frame *frame)
{
    cairo_surface_t *surface = NULL;
    cairo_t *cr = NULL;
    gchar *path;
    gdouble draw_width;
    gdouble draw_height;
    gboolean success;

    path = g_strdup_printf ("%s.%s", base_filename, export_to);
    if (!begin_export_surface (tbo,
                               export_to,
                               path,
                               width,
                               height,
                               FALSE,
                               &surface,
                               &cr,
                               &draw_width,
                               &draw_height))
    {
        g_free (path);
        return FALSE;
    }

    draw_frame_export (cr, frame, (gint) draw_width, (gint) draw_height);
    success = finish_export_surface (tbo, export_to, path, surface, cr);

    cairo_surface_destroy (surface);
    cairo_destroy (cr);
    g_free (path);
    return success;
}

gboolean
tbo_export_file_with_scope_range (TboWindow *tbo,
                                  const gchar *filename,
                                  const gchar *format_hint,
                                  gint width,
                                  gint height,
                                  TboExportScope scope,
                                  gint from_page,
                                  gint to_page)
{
    gchar *base_filename = NULL;
    gchar *export_to = NULL;
    GList *pages = NULL;
    gint n_pages = 0;
    gboolean success = FALSE;

    if (filename == NULL || *filename == '\0' || width <= 0 || height <= 0)
        return FALSE;

    if (format_hint != NULL && *format_hint != '\0')
    {
        export_to = g_ascii_strdown (format_hint, -1);
        base_filename = strip_matching_extension (filename, export_to);
    }
    else
    {
        gchar *dot = strrchr (filename, '.');

        if (dot != NULL && dot[1] != '\0')
        {
            export_to = g_ascii_strdown (dot + 1, -1);
            base_filename = g_strndup (filename, dot - filename);
        }
        else
        {
            base_filename = g_strdup (filename);
            export_to = g_strdup ("png");
        }
    }

    if (g_strcmp0 (export_to, "png") != 0 &&
        g_strcmp0 (export_to, "pdf") != 0 &&
        g_strcmp0 (export_to, "svg") != 0)
    {
        g_free (export_to);
        export_to = g_strdup ("png");
    }

    switch (scope)
    {
        case TBO_EXPORT_SCOPE_CURRENT_PAGE:
        {
            Page *current_page = tbo_comic_get_current_page (tbo->comic);

            if (current_page == NULL)
            {
                show_export_error (tbo, _("There are no pages to export."));
                success = FALSE;
            }
            else
            {
                pages = g_list_append (NULL, current_page);
                success = export_page_list (tbo, base_filename, export_to, width, height, pages, 1, TRUE);
                g_list_free (pages);
            }
            break;
        }
        case TBO_EXPORT_SCOPE_SELECTION:
        {
            Frame *frame = get_export_selection_frame (tbo);

            if (frame == NULL)
            {
                show_export_error (tbo, _("Please select a frame to export."));
                success = FALSE;
            }
            else
            {
                success = export_single_frame (tbo, base_filename, export_to, width, height, frame);
            }
            break;
        }
        case TBO_EXPORT_SCOPE_ALL_PAGES:
        default:
            pages = build_export_page_range (tbo->comic, from_page, to_page, &n_pages);
            success = export_page_list (tbo,
                                        base_filename,
                                        export_to,
                                        width,
                                        height,
                                        pages,
                                        n_pages,
                                        TRUE);
            g_list_free (pages);
            break;
    }

    g_free (base_filename);
    g_free (export_to);
    return success;
}

gboolean
tbo_export_file_with_scope (TboWindow *tbo,
                            const gchar *filename,
                            const gchar *format_hint,
                            gint width,
                            gint height,
                            TboExportScope scope)
{
    return tbo_export_file_with_scope_range (tbo,
                                             filename,
                                             format_hint,
                                             width,
                                             height,
                                             scope,
                                             1,
                                             tbo_comic_len (tbo->comic));
}

gboolean
tbo_export_file (TboWindow *tbo,
                 const gchar *filename,
                 const gchar *format_hint,
                 gint width,
                 gint height)
{
    return tbo_export_file_with_scope (tbo,
                                       filename,
                                       format_hint,
                                       width,
                                       height,
                                       TBO_EXPORT_SCOPE_ALL_PAGES);
}

gboolean
tbo_export (TboWindow *tbo)
{
    gint width = tbo_comic_get_width (tbo->comic);
    gint height = tbo_comic_get_height (tbo->comic);
    gint selection_width = width;
    gint selection_height = height;
    gint page_count = tbo_comic_len (tbo->comic);
    gchar *filename = NULL;
    gint response;
    gint export_to_index;
    gint from_page;
    gint to_page;
    TboExportScope scope;
    ExportFileArgs file_args;
    ExportDialogState dialog_state;

    GtkWidget *dialog;
    GtkWidget *headerbar;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *fileinput;
    GtkWidget *filelabel;
    GtkWidget *filebutton;
    GtkWidget *spinw;
    GtkWidget *spinh;
    GtkWidget *format_dropdown;
    GtkWidget *scope_dropdown;
    GtkWidget *range_row;
    GtkWidget *range_from_label;
    GtkWidget *range_from_spin;
    GtkWidget *range_to_label;
    GtkWidget *range_to_spin;
    GtkWidget *preview_frame;
    GtkWidget *preview_vbox;
    GtkWidget *preview_label;
    GtkWidget *preview_box;
    GtkWidget *actions;
    GtkWidget *button;
    GtkWidget *scope_label;
    gchar *basename = NULL;
    const char *export_formats[] = {
        "Guess by Extension",
        ".png",
        ".pdf",
        ".svg",
        NULL,
    };
    const char *export_scopes_with_selection[] = {
        _("All Pages"),
        _("Current Page"),
        _("Selection"),
        NULL,
    };
    const char *export_scopes_without_selection[] = {
        _("All Pages"),
        _("Current Page"),
        NULL,
    };
    TboDialogRunData data;
    const gchar *format_hint = NULL;
    gboolean has_selection = has_export_selection (tbo);

    get_export_scope_default_size (tbo, TBO_EXPORT_SCOPE_SELECTION, &selection_width, &selection_height);

    dialog = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (dialog), _("Export"));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (tbo->window));
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_window_set_default_size (GTK_WINDOW (dialog), 420, -1);

    headerbar = gtk_header_bar_new ();
    gtk_header_bar_set_show_title_buttons (GTK_HEADER_BAR (headerbar), TRUE);
    gtk_window_set_titlebar (GTK_WINDOW (dialog), headerbar);

    filebutton = gtk_button_new_with_label (_("Choose File"));
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class (vbox, "tbo-dialog-content");
    gtk_widget_set_margin_start (vbox, 12);
    gtk_widget_set_margin_end (vbox, 12);
    gtk_widget_set_margin_top (vbox, 12);
    gtk_widget_set_margin_bottom (vbox, 12);
    tbo_widget_add_child (dialog, vbox);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    filelabel = gtk_label_new (_("File Name: "));
    fileinput = gtk_entry_new ();
    if (tbo->export_path != NULL)
    {
        basename = g_path_get_basename (tbo->export_path);
        gtk_editable_set_text (GTK_EDITABLE (fileinput), basename);
        g_free (basename);
    }
    else
    {
        gtk_editable_set_text (GTK_EDITABLE (fileinput), tbo_comic_get_title (tbo->comic));
    }
    tbo_widget_add_child (hbox, filelabel);
    tbo_widget_add_child (hbox, fileinput);
    tbo_widget_add_child (hbox, filebutton);
    tbo_widget_add_child (vbox, hbox);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    scope_label = gtk_label_new (_("Scope: "));
    gtk_widget_set_size_request (scope_label, 80, -1);
    gtk_label_set_xalign (GTK_LABEL (scope_label), 0.0);
    scope_dropdown = gtk_drop_down_new_from_strings (has_selection ?
                                                     export_scopes_with_selection :
                                                     export_scopes_without_selection);
    gtk_widget_set_name (scope_dropdown, "export-scope");
    gtk_drop_down_set_selected (GTK_DROP_DOWN (scope_dropdown), 0);
    tbo_widget_add_child (hbox, scope_label);
    tbo_widget_add_child (hbox, scope_dropdown);
    tbo_widget_add_child (vbox, hbox);

    spinw = add_spin_with_label (vbox, _("Width: "), width);
    spinh = add_spin_with_label (vbox, _("Height: "), height);
    gtk_widget_set_name (spinw, "export-width");
    gtk_widget_set_name (spinh, "export-height");

    range_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    range_from_label = gtk_label_new (_("From Page: "));
    gtk_widget_set_size_request (range_from_label, 80, -1);
    gtk_label_set_xalign (GTK_LABEL (range_from_label), 0.0);
    range_from_spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, page_count, 1, 1, 0)), 1, 0);
    gtk_widget_set_name (range_from_spin, "export-range-from");
    range_to_label = gtk_label_new (_("To Page: "));
    gtk_label_set_xalign (GTK_LABEL (range_to_label), 0.0);
    range_to_spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (page_count, 1, page_count, 1, 1, 0)), 1, 0);
    gtk_widget_set_name (range_to_spin, "export-range-to");
    tbo_widget_add_child (range_row, range_from_label);
    tbo_widget_add_child (range_row, range_from_spin);
    tbo_widget_add_child (range_row, range_to_label);
    tbo_widget_add_child (range_row, range_to_spin);
    tbo_widget_add_child (vbox, range_row);

    format_dropdown = gtk_drop_down_new_from_strings (export_formats);
    gtk_widget_set_name (format_dropdown, "export-format");
    gtk_drop_down_set_selected (GTK_DROP_DOWN (format_dropdown), 0);
    tbo_widget_add_child (vbox, format_dropdown);

    preview_frame = gtk_frame_new (_("Preview"));
    gtk_widget_add_css_class (preview_frame, "tbo-dialog-card");
    preview_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_start (preview_vbox, 8);
    gtk_widget_set_margin_end (preview_vbox, 8);
    gtk_widget_set_margin_top (preview_vbox, 8);
    gtk_widget_set_margin_bottom (preview_vbox, 8);
    preview_label = gtk_label_new (NULL);
    gtk_widget_set_name (preview_label, "export-preview-label");
    gtk_label_set_xalign (GTK_LABEL (preview_label), 0.0);
    preview_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name (preview_box, "export-preview-box");
    gtk_widget_set_size_request (preview_box, 220, 160);
    tbo_widget_add_child (preview_vbox, preview_label);
    tbo_widget_add_child (preview_vbox, preview_box);
    tbo_widget_add_child (preview_frame, preview_vbox);
    tbo_widget_add_child (vbox, preview_frame);

    dialog_state.tbo = tbo;
    dialog_state.scope_dropdown = scope_dropdown;
    dialog_state.format_dropdown = format_dropdown;
    dialog_state.range_row = range_row;
    dialog_state.range_from_spin = range_from_spin;
    dialog_state.range_to_spin = range_to_spin;
    dialog_state.preview_box = preview_box;
    dialog_state.preview_label = preview_label;
    dialog_state.page_width = width;
    dialog_state.page_height = height;
    dialog_state.selection_width = selection_width;
    dialog_state.selection_height = selection_height;
    dialog_state.has_selection = has_selection;
    dialog_state.size_state.spinw = spinw;
    dialog_state.size_state.spinh = spinh;
    dialog_state.size_state.base_width = width;
    dialog_state.size_state.base_height = height;
    dialog_state.size_state.updating = FALSE;

    g_signal_connect (spinw, "value-changed", G_CALLBACK (dialog_width_changed_cb), &dialog_state);
    g_signal_connect (spinh, "value-changed", G_CALLBACK (dialog_height_changed_cb), &dialog_state);
    g_signal_connect (scope_dropdown, "notify::selected", G_CALLBACK (scope_selected_cb), &dialog_state);
    g_signal_connect (format_dropdown, "notify::selected", G_CALLBACK (format_selected_cb), &dialog_state);
    g_signal_connect (range_from_spin, "value-changed", G_CALLBACK (range_from_changed_cb), &dialog_state);
    g_signal_connect (range_to_spin, "value-changed", G_CALLBACK (range_to_changed_cb), &dialog_state);
    set_export_size_base (&dialog_state.size_state, width, height);
    update_preview_and_range (&dialog_state);

    actions = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign (actions, GTK_ALIGN_END);

    button = gtk_button_new_with_mnemonic (_("_Cancel"));
    g_object_set_data (G_OBJECT (button), "tbo-response", GINT_TO_POINTER (GTK_RESPONSE_CANCEL));
    g_signal_connect (button, "clicked", G_CALLBACK (tbo_dialog_button_cb), dialog);
    tbo_widget_add_child (actions, button);

    button = gtk_button_new_with_mnemonic (_("_Save"));
    gtk_widget_add_css_class (button, "suggested-action");
    g_object_set_data (G_OBJECT (button), "tbo-response", GINT_TO_POINTER (GTK_RESPONSE_ACCEPT));
    g_signal_connect (button, "clicked", G_CALLBACK (tbo_dialog_button_cb), dialog);
    tbo_widget_add_child (actions, button);

    tbo_widget_add_child (vbox, actions);
    tbo_widget_show_all (GTK_WIDGET (vbox));

    file_args.tbo = tbo;
    file_args.entry = GTK_ENTRY (fileinput);
    g_signal_connect (filebutton, "clicked", G_CALLBACK (filedialog_cb), &file_args);

    tbo_dialog_run_data_init (&data, GTK_RESPONSE_CANCEL);
    g_signal_connect (dialog, "close-request", G_CALLBACK (tbo_dialog_close_request_cb), &data);
    tbo_dialog_run (GTK_WINDOW (dialog), &data);

    response = data.response;

    if (response == GTK_RESPONSE_ACCEPT)
    {
        width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinw));
        height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinh));
        scope = dropdown_scope_to_export_scope (gtk_drop_down_get_selected (GTK_DROP_DOWN (scope_dropdown)), has_selection);
        get_dialog_range (&dialog_state, &from_page, &to_page);

        filename = g_strdup (gtk_editable_get_text (GTK_EDITABLE (fileinput)));
        if (filename == NULL || *filename == '\0')
        {
            show_export_error (tbo, _("Please choose a filename to export."));
            g_free (filename);
            gtk_window_destroy (GTK_WINDOW (dialog));
            return FALSE;
        }

        tbo_window_set_export_path (tbo, filename);
        export_to_index = gtk_drop_down_get_selected (GTK_DROP_DOWN (format_dropdown));
        if (export_to_index == 1)
            format_hint = "png";
        else if (export_to_index == 2)
            format_hint = "pdf";
        else if (export_to_index == 3)
            format_hint = "svg";

        if (!tbo_export_file_with_scope_range (tbo, filename, format_hint, width, height, scope, from_page, to_page))
        {
            gtk_window_destroy (GTK_WINDOW (dialog));
            tbo_dialog_run_data_clear (&data);
            g_free (filename);
            return FALSE;
        }
    }

    g_free (filename);
    gtk_window_destroy (GTK_WINDOW (dialog));
    tbo_dialog_run_data_clear (&data);
    return response == GTK_RESPONSE_ACCEPT;
}
