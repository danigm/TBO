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


#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comic-load.h"
#include "tbo-widget.h"
#include "tbo-types.h"
#include "comic.h"
#include "page.h"
#include "frame.h"
#include "tbo-object-svg.h"
#include "tbo-object-text.h"
#include "tbo-object-pixmap.h"
#include "tbo-utils.h"

typedef enum
{
    ATTR_INT,
    ATTR_DOUBLE,
} TboLoadAttrType;

typedef struct
{
    const gchar *name;
    TboLoadAttrType type;
    gpointer pointer;
    gboolean required;
    gboolean seen;
} TboLoadAttr;

typedef struct
{
    gchar *title;
    Comic *comic;
    Page *current_page;
    Frame *current_frame;
    TboObjectText *current_text;
    GString *current_text_buffer;
} TboLoadContext;

static gchar *
unwrap_saved_text (const gchar *text)
{
    gsize start = 0;
    gsize end;
    gsize trailing;

    if (text == NULL)
        return g_strdup ("");

    end = strlen (text);
    if (end > 0 && text[0] == '\n')
        start = 1;

    trailing = end;
    while (trailing > start && (text[trailing - 1] == ' ' || text[trailing - 1] == '\t'))
        trailing--;

    if (trailing > start && text[trailing - 1] == '\n')
        end = trailing - 1;

    return g_strndup (text + start, end - start);
}

static const gchar *
find_attr_value (const gchar **attribute_names,
                 const gchar **attribute_values,
                 const gchar *name)
{
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;

    while (*name_cursor != NULL)
    {
        if (strcmp (*name_cursor, name) == 0)
            return *value_cursor;
        name_cursor++;
        value_cursor++;
    }

    return NULL;
}

static gchar *
dup_required_attr_string (const gchar **attribute_names,
                          const gchar **attribute_values,
                          const gchar *element_name,
                          const gchar *attr_name,
                          GError **error)
{
    const gchar *value = find_attr_value (attribute_names, attribute_values, attr_name);

    if (value == NULL || *value == '\0')
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Element '%s' is missing required attribute '%s'",
                     element_name,
                     attr_name);
        return NULL;
    }

    return g_strdup (value);
}

static gboolean
parse_attrs (const gchar *element_name,
             TboLoadAttr attrs[],
             gsize attrs_size,
             const gchar **attribute_names,
             const gchar **attribute_values,
             GError **error)
{
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;
    gsize i;

    for (i = 0; i < attrs_size; i++)
        attrs[i].seen = FALSE;

    while (*name_cursor != NULL)
    {
        for (i = 0; i < attrs_size; i++)
        {
            if (strcmp (*name_cursor, attrs[i].name) != 0)
                continue;

            attrs[i].seen = TRUE;
            if (attrs[i].type == ATTR_INT)
            {
                if (!tbo_ascii_parse_int (*value_cursor, attrs[i].pointer))
                {
                    g_set_error (error,
                                 G_MARKUP_ERROR,
                                 G_MARKUP_ERROR_INVALID_CONTENT,
                                 "Invalid integer value '%s' for '%s' attribute '%s'",
                                 *value_cursor,
                                 element_name,
                                 attrs[i].name);
                    return FALSE;
                }
            }
            else if (attrs[i].type == ATTR_DOUBLE)
            {
                if (!tbo_ascii_parse_double (*value_cursor, attrs[i].pointer))
                {
                    g_set_error (error,
                                 G_MARKUP_ERROR,
                                 G_MARKUP_ERROR_INVALID_CONTENT,
                                 "Invalid decimal value '%s' for '%s' attribute '%s'",
                                 *value_cursor,
                                 element_name,
                                 attrs[i].name);
                    return FALSE;
                }
            }
            break;
        }

        name_cursor++;
        value_cursor++;
    }

    for (i = 0; i < attrs_size; i++)
    {
        if (attrs[i].required && !attrs[i].seen)
        {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_INVALID_CONTENT,
                         "Element '%s' is missing required attribute '%s'",
                         element_name,
                         attrs[i].name);
            return FALSE;
        }
    }

    return TRUE;
}

static gboolean
create_tbo_comic (TboLoadContext *context,
                  const gchar **attribute_names,
                  const gchar **attribute_values,
                  GError **error)
{
    gint width = 0;
    gint height = 0;
    const gchar *paper_value;
    TboComicPaper paper = TBO_COMIC_PAPER_NONE;
    TboLoadAttr attrs[] = {
        {"width", ATTR_INT, &width, TRUE, FALSE},
        {"height", ATTR_INT, &height, TRUE, FALSE},
    };

    if (context->comic != NULL)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Element 'tbo' appears more than once");
        return FALSE;
    }

    if (!parse_attrs ("tbo", attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values, error))
        return FALSE;

    if (width <= 0 || height <= 0)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Comic size must be positive");
        return FALSE;
    }

    paper_value = find_attr_value (attribute_names, attribute_values, "paper");
    if (paper_value != NULL && *paper_value != '\0')
    {
        if (g_ascii_strcasecmp (paper_value, "a4") == 0)
            paper = TBO_COMIC_PAPER_A4;
        else
        {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_INVALID_CONTENT,
                         "Unsupported paper value '%s'",
                         paper_value);
            return FALSE;
        }
    }

    context->comic = tbo_comic_new (context->title, width, height);
    tbo_comic_set_paper (context->comic, paper);
    tbo_comic_del_page (context->comic, 0);
    return TRUE;
}

static gboolean
create_tbo_page (TboLoadContext *context, GError **error)
{
    if (context->comic == NULL)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Element 'page' must be inside 'tbo'");
        return FALSE;
    }

    context->current_page = tbo_comic_new_page (context->comic);
    context->current_frame = NULL;
    return TRUE;
}

static gboolean
create_tbo_frame (TboLoadContext *context,
                  const gchar **attribute_names,
                  const gchar **attribute_values,
                  GError **error)
{
    gint x = 0;
    gint y = 0;
    gint width = 0;
    gint height = 0;
    gint border = 1;
    gdouble r = 0.0;
    gdouble g = 0.0;
    gdouble b = 0.0;
    TboLoadAttr attrs[] = {
        {"x", ATTR_INT, &x, TRUE, FALSE},
        {"y", ATTR_INT, &y, TRUE, FALSE},
        {"width", ATTR_INT, &width, TRUE, FALSE},
        {"height", ATTR_INT, &height, TRUE, FALSE},
        {"border", ATTR_INT, &border, FALSE, FALSE},
        {"r", ATTR_DOUBLE, &r, FALSE, FALSE},
        {"g", ATTR_DOUBLE, &g, FALSE, FALSE},
        {"b", ATTR_DOUBLE, &b, FALSE, FALSE},
    };

    if (context->current_page == NULL)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Element 'frame' must be inside 'page'");
        return FALSE;
    }

    if (!parse_attrs ("frame", attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values, error))
        return FALSE;

    if (width <= 0 || height <= 0)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Frame size must be positive");
        return FALSE;
    }

    context->current_frame = tbo_page_new_frame (context->current_page, x, y, width, height);
    tbo_frame_set_border (context->current_frame, border != 0);
    tbo_frame_set_color_rgb (context->current_frame, r, g, b);
    return TRUE;
}

static gboolean
create_tbo_piximage (TboLoadContext *context,
                     const gchar **attribute_names,
                     const gchar **attribute_values,
                     GError **error)
{
    TboObjectPixmap *pix;
    TboObjectBase *obj;
    gint x = 0;
    gint y = 0;
    gint width = 0;
    gint height = 0;
    gint flipv = 0;
    gint fliph = 0;
    gdouble angle = 0.0;
    gchar *path;
    TboLoadAttr attrs[] = {
        {"x", ATTR_INT, &x, TRUE, FALSE},
        {"y", ATTR_INT, &y, TRUE, FALSE},
        {"width", ATTR_INT, &width, TRUE, FALSE},
        {"height", ATTR_INT, &height, TRUE, FALSE},
        {"flipv", ATTR_INT, &flipv, FALSE, FALSE},
        {"fliph", ATTR_INT, &fliph, FALSE, FALSE},
        {"angle", ATTR_DOUBLE, &angle, FALSE, FALSE},
    };

    if (context->current_frame == NULL)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Element 'piximage' must be inside 'frame'");
        return FALSE;
    }

    if (!parse_attrs ("piximage", attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values, error))
        return FALSE;

    if (width < 0 || height < 0)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "piximage size cannot be negative");
        return FALSE;
    }

    path = dup_required_attr_string (attribute_names, attribute_values, "piximage", "path", error);
    if (path == NULL)
        return FALSE;

    pix = TBO_OBJECT_PIXMAP (tbo_object_pixmap_new_with_params (x, y, width, height, path));
    obj = TBO_OBJECT_BASE (pix);
    obj->angle = angle;
    obj->flipv = flipv;
    obj->fliph = fliph;
    tbo_frame_add_obj (context->current_frame, obj);
    g_free (path);
    return TRUE;
}

static gboolean
create_tbo_svgimage (TboLoadContext *context,
                     const gchar **attribute_names,
                     const gchar **attribute_values,
                     GError **error)
{
    TboObjectSvg *svg;
    TboObjectBase *obj;
    gint x = 0;
    gint y = 0;
    gint width = 0;
    gint height = 0;
    gint flipv = 0;
    gint fliph = 0;
    gdouble angle = 0.0;
    gchar *path;
    TboLoadAttr attrs[] = {
        {"x", ATTR_INT, &x, TRUE, FALSE},
        {"y", ATTR_INT, &y, TRUE, FALSE},
        {"width", ATTR_INT, &width, TRUE, FALSE},
        {"height", ATTR_INT, &height, TRUE, FALSE},
        {"flipv", ATTR_INT, &flipv, FALSE, FALSE},
        {"fliph", ATTR_INT, &fliph, FALSE, FALSE},
        {"angle", ATTR_DOUBLE, &angle, FALSE, FALSE},
    };

    if (context->current_frame == NULL)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Element 'svgimage' must be inside 'frame'");
        return FALSE;
    }

    if (!parse_attrs ("svgimage", attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values, error))
        return FALSE;

    if (width < 0 || height < 0)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "svgimage size cannot be negative");
        return FALSE;
    }

    path = dup_required_attr_string (attribute_names, attribute_values, "svgimage", "path", error);
    if (path == NULL)
        return FALSE;

    svg = TBO_OBJECT_SVG (tbo_object_svg_new_with_params (x, y, width, height, path));
    obj = TBO_OBJECT_BASE (svg);
    obj->angle = angle;
    obj->flipv = flipv;
    obj->fliph = fliph;
    tbo_frame_add_obj (context->current_frame, obj);
    g_free (path);
    return TRUE;
}

static gboolean
create_tbo_text (TboLoadContext *context,
                 const gchar **attribute_names,
                 const gchar **attribute_values,
                 GError **error)
{
    TboObjectText *textobj;
    TboObjectBase *obj;
    GdkRGBA color = { 0, 0, 0, 1 };
    gint x = 0;
    gint y = 0;
    gint width = 0;
    gint height = 0;
    gint flipv = 0;
    gint fliph = 0;
    gdouble angle = 0.0;
    gdouble r = 0.0;
    gdouble g = 0.0;
    gdouble b = 0.0;
    gchar *font;
    TboLoadAttr attrs[] = {
        {"x", ATTR_INT, &x, TRUE, FALSE},
        {"y", ATTR_INT, &y, TRUE, FALSE},
        {"width", ATTR_INT, &width, TRUE, FALSE},
        {"height", ATTR_INT, &height, TRUE, FALSE},
        {"flipv", ATTR_INT, &flipv, FALSE, FALSE},
        {"fliph", ATTR_INT, &fliph, FALSE, FALSE},
        {"angle", ATTR_DOUBLE, &angle, FALSE, FALSE},
        {"r", ATTR_DOUBLE, &r, FALSE, FALSE},
        {"g", ATTR_DOUBLE, &g, FALSE, FALSE},
        {"b", ATTR_DOUBLE, &b, FALSE, FALSE},
    };

    if (context->current_frame == NULL)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "Element 'text' must be inside 'frame'");
        return FALSE;
    }

    if (!parse_attrs ("text", attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values, error))
        return FALSE;

    if (width < 0 || height < 0)
    {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_INVALID_CONTENT,
                     "text size cannot be negative");
        return FALSE;
    }

    font = dup_required_attr_string (attribute_names, attribute_values, "text", "font", error);
    if (font == NULL)
        return FALSE;

    color.red = r;
    color.green = g;
    color.blue = b;

    textobj = TBO_OBJECT_TEXT (tbo_object_text_new_with_params (x, y, width, height, "", font, &color));
    obj = TBO_OBJECT_BASE (textobj);
    obj->angle = angle;
    obj->flipv = flipv;
    obj->fliph = fliph;

    context->current_text = textobj;
    if (context->current_text_buffer != NULL)
        g_string_free (context->current_text_buffer, TRUE);
    context->current_text_buffer = g_string_new (NULL);

    tbo_frame_add_obj (context->current_frame, obj);
    g_free (font);
    return TRUE;
}

static void
start_element (GMarkupParseContext *markup_context,
               const gchar *element_name,
               const gchar **attribute_names,
               const gchar **attribute_values,
               gpointer user_data,
               GError **error)
{
    TboLoadContext *context = user_data;

    if (strcmp (element_name, "tbo") == 0)
    {
        create_tbo_comic (context, attribute_names, attribute_values, error);
    }
    else if (strcmp (element_name, "page") == 0)
    {
        create_tbo_page (context, error);
    }
    else if (strcmp (element_name, "frame") == 0)
    {
        create_tbo_frame (context, attribute_names, attribute_values, error);
    }
    else if (strcmp (element_name, "svgimage") == 0)
    {
        create_tbo_svgimage (context, attribute_names, attribute_values, error);
    }
    else if (strcmp (element_name, "piximage") == 0)
    {
        create_tbo_piximage (context, attribute_names, attribute_values, error);
    }
    else if (strcmp (element_name, "text") == 0)
    {
        create_tbo_text (context, attribute_names, attribute_values, error);
    }

    (void) markup_context;
}

static void
text_element (GMarkupParseContext *markup_context,
              const gchar *text,
              gsize text_len,
              gpointer user_data,
              GError **error)
{
    TboLoadContext *context = user_data;

    if (context->current_text_buffer != NULL)
        g_string_append_len (context->current_text_buffer, text, text_len);

    (void) markup_context;
    (void) error;
}

static void
end_element (GMarkupParseContext *markup_context,
             const gchar *element_name,
             gpointer user_data,
             GError **error)
{
    TboLoadContext *context = user_data;

    if (strcmp (element_name, "text") == 0)
    {
        gchar *normalized = NULL;

        if (context->current_text != NULL && context->current_text_buffer != NULL)
        {
            normalized = unwrap_saved_text (context->current_text_buffer->str);
            tbo_object_text_set_text (context->current_text, normalized);
        }

        g_free (normalized);
        if (context->current_text_buffer != NULL)
        {
            g_string_free (context->current_text_buffer, TRUE);
            context->current_text_buffer = NULL;
        }
        context->current_text = NULL;
    }

    (void) markup_context;
    (void) error;
}

static GMarkupParser parser = {
    start_element,
    end_element,
    text_element,
    NULL,
    NULL
};

Comic *
tbo_comic_load_with_alerts (const gchar *filename, gboolean show_alerts)
{
    TboLoadContext context = { 0 };
    GMarkupParseContext *markup_context;
    GError *error = NULL;
    gchar *file_text = NULL;
    gsize length = 0;
    gchar base_name[255];
    Comic *comic;

    markup_context = g_markup_parse_context_new (&parser, 0, &context, NULL);

    get_base_name (filename, base_name, 255);
    context.title = g_strdup (base_name);

    if (!g_file_get_contents (filename, &file_text, &length, &error))
    {
        if (show_alerts)
            tbo_alert_show (NULL, _("Couldn't load file"), error != NULL ? error->message : NULL);
        g_clear_error (&error);
        g_markup_parse_context_free (markup_context);
        g_free (context.title);
        return NULL;
    }

    if (!g_markup_parse_context_parse (markup_context, file_text, length, &error) ||
        !g_markup_parse_context_end_parse (markup_context, &error))
    {
        if (show_alerts)
            tbo_alert_show (NULL, _("Couldn't parse file"), error != NULL ? error->message : NULL);
        g_clear_error (&error);
        if (context.current_text_buffer != NULL)
            g_string_free (context.current_text_buffer, TRUE);
        if (context.comic != NULL)
            tbo_comic_free (context.comic);
        g_markup_parse_context_free (markup_context);
        g_free (file_text);
        g_free (context.title);
        return NULL;
    }

    if (context.comic == NULL)
    {
        if (show_alerts)
            tbo_alert_show (NULL, _("Couldn't parse file"), _("No comic data found in file"));
        g_markup_parse_context_free (markup_context);
        g_free (file_text);
        g_free (context.title);
        return NULL;
    }

    if (tbo_comic_len (context.comic) == 0)
    {
        if (show_alerts)
            tbo_alert_show (NULL, _("Couldn't parse file"), _("No pages found in file"));
        tbo_comic_free (context.comic);
        if (context.current_text_buffer != NULL)
            g_string_free (context.current_text_buffer, TRUE);
        g_markup_parse_context_free (markup_context);
        g_free (file_text);
        g_free (context.title);
        return NULL;
    }

    comic = context.comic;
    context.comic = NULL;

    if (context.current_text_buffer != NULL)
        g_string_free (context.current_text_buffer, TRUE);
    g_markup_parse_context_free (markup_context);
    g_free (file_text);
    g_free (context.title);

    return comic;
}

Comic *
tbo_comic_load (const gchar *filename)
{
    return tbo_comic_load_with_alerts (filename, TRUE);
}
