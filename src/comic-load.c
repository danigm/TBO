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
#include "tbo-types.h"
#include "comic.h"
#include "page.h"
#include "frame.h"
#include "tbo-object-svg.h"
#include "tbo-object-text.h"
#include "tbo-object-pixmap.h"
#include "tbo-utils.h"

char *TITLE;

Comic *COMIC = NULL;
Page *CURRENT_PAGE;
Frame *CURRENT_FRAME;
TboObjectText *CURRENT_TEXT = NULL;

struct attr {
    char *name;
    char *format;
    void *pointer;
};

void
parse_attrs (struct attr attrs[],
             int attrs_size,
             const gchar **attribute_names,
             const gchar **attribute_values)
{
    int i;
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;

    while (*name_cursor) {
        for (i=0; i<attrs_size; i++)
        {
            if (strcmp (*name_cursor, attrs[i].name) == 0)
            {
                if (strcmp (attrs[i].format, "%s") == 0)
                {
                    sprintf(attrs[i].pointer, "%s", *value_cursor);
                }
                else
                    sscanf (*value_cursor, attrs[i].format, attrs[i].pointer);
            }
        }
        name_cursor++;
        value_cursor++;
    }
}

void
create_tbo_comic (const gchar **attribute_names, const gchar **attribute_values)
{
    int width = 0;
    int height = 0;

    struct attr attrs[] = {
        {"width", "%d", &width},
        {"height", "%d", &height},
    };

    parse_attrs (attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values);

    COMIC = tbo_comic_new (TITLE, width, height);
    tbo_comic_del_page (COMIC, 0);
}

void
create_tbo_frame (const gchar **attribute_names, const gchar **attribute_values)
{
    int x=0, y=0;
    int width=0, height=0;
    int border=1;
    float r=0.0, g=0.0, b=0.0;

    struct attr attrs[] = {
        {"x", "%d", &x},
        {"y", "%d", &y},
        {"width", "%d", &width},
        {"height", "%d", &height},
        {"border", "%d", &border},
        {"r", "%f", &r},
        {"g", "%f", &g},
        {"b", "%f", &b},
    };

    parse_attrs (attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values);

    CURRENT_FRAME = tbo_page_new_frame (CURRENT_PAGE, x, y, width, height);
    CURRENT_FRAME->border = border;
    CURRENT_FRAME->color->r = r;
    CURRENT_FRAME->color->g = g;
    CURRENT_FRAME->color->b = b;
}

void
create_tbo_piximage (const gchar **attribute_names, const gchar **attribute_values)
{
    TboObjectPixmap *pix;
    TboObjectBase *obj;
    int x=0, y=0;
    int width=0, height=0;
    float angle=0.0;
    int flipv=0, fliph=0;
    char path[255];

    struct attr attrs[] = {
        {"x", "%d", &x},
        {"y", "%d", &y},
        {"width", "%d", &width},
        {"height", "%d", &height},
        {"flipv", "%d", &flipv},
        {"fliph", "%d", &fliph},
        {"angle", "%f", &angle},
        {"path", "%s", path},
    };

    parse_attrs (attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values);

    pix = TBO_OBJECT_PIXMAP (tbo_object_pixmap_new_with_params (x, y, width, height, path));
    obj = TBO_OBJECT_BASE (pix);
    obj->angle = angle;
    obj->flipv = flipv;
    obj->fliph = fliph;
    tbo_frame_add_obj (CURRENT_FRAME, obj);
}

void
create_tbo_svgimage (const gchar **attribute_names, const gchar **attribute_values)
{
    TboObjectSvg *svg;
    TboObjectBase *obj;
    int x=0, y=0;
    int width=0, height=0;
    float angle=0.0;
    int flipv=0, fliph=0;
    char path[255];

    struct attr attrs[] = {
        {"x", "%d", &x},
        {"y", "%d", &y},
        {"width", "%d", &width},
        {"height", "%d", &height},
        {"flipv", "%d", &flipv},
        {"fliph", "%d", &fliph},
        {"angle", "%f", &angle},
        {"path", "%s", path},
    };

    parse_attrs (attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values);

    svg = TBO_OBJECT_SVG (tbo_object_svg_new_with_params (x, y, width, height, path));
    obj = TBO_OBJECT_BASE (svg);
    obj->angle = angle;
    obj->flipv = flipv;
    obj->fliph = fliph;
    tbo_frame_add_obj (CURRENT_FRAME, obj);
}

void
create_tbo_text (const gchar **attribute_names, const gchar **attribute_values)
{
    TboObjectText *textobj;
    TboObjectBase *obj;
    GdkColor color;
    int x=0, y=0;
    int width=0, height=0;
    float angle=0.0;
    int flipv=0, fliph=0;
    char font[255];
    float r=0.0, g=0.0, b=0.0;

    struct attr attrs[] = {
        {"x", "%d", &x},
        {"y", "%d", &y},
        {"width", "%d", &width},
        {"height", "%d", &height},
        {"flipv", "%d", &flipv},
        {"fliph", "%d", &fliph},
        {"angle", "%f", &angle},
        {"font", "%s", font},
        {"r", "%f", &r},
        {"g", "%f", &g},
        {"b", "%f", &b},
    };

    parse_attrs (attrs, G_N_ELEMENTS (attrs), attribute_names, attribute_values);
    color.red = (int)(r * COLORMAX);
    color.green = (int)(g * COLORMAX);
    color.blue = (int)(b * COLORMAX);
    textobj = TBO_OBJECT_TEXT (tbo_object_text_new_with_params (x, y, width, height, "text", font, &color));
    obj = TBO_OBJECT_BASE (textobj);
    obj->angle = angle;
    obj->flipv = flipv;
    obj->fliph = fliph;
    CURRENT_TEXT = textobj;
    tbo_frame_add_obj (CURRENT_FRAME, obj);
}

/* The handler functions. */

void
start_element (GMarkupParseContext *context,
               const gchar         *element_name,
               const gchar        **attribute_names,
               const gchar        **attribute_values,
               gpointer             user_data,
               GError             **error)
{

    if (strcmp (element_name, "tbo") == 0)
    {
        create_tbo_comic (attribute_names, attribute_values);
    }
    else if (strcmp (element_name, "page") == 0)
    {
        CURRENT_PAGE = tbo_comic_new_page (COMIC);
    }
    else if (strcmp (element_name, "frame") == 0)
    {
        create_tbo_frame (attribute_names, attribute_values);
    }
    else if (strcmp (element_name, "svgimage") == 0)
    {
        create_tbo_svgimage (attribute_names, attribute_values);
    }
    else if (strcmp (element_name, "piximage") == 0)
    {
        create_tbo_piximage (attribute_names, attribute_values);
    }
    else if (strcmp (element_name, "text") == 0)
    {
        create_tbo_text (attribute_names, attribute_values);
    }
}

void
text (GMarkupParseContext *context,
        const gchar       *text,
        gsize              text_len,
        gpointer           user_data,
        GError            **error)
{
    if (CURRENT_TEXT)
    {
        char *text2 = g_strndup (text, text_len);
        tbo_object_text_set_text (CURRENT_TEXT, g_strstrip (text2));
        g_free (text2);
    }
}

void
end_element (GMarkupParseContext *context,
             const gchar         *element_name,
             gpointer             user_data,
             GError             **error)
{
    if (strcmp (element_name, "tbo") == 0)
    {
        g_free (TITLE);
    }
    else if (strcmp (element_name, "text") == 0)
    {
        CURRENT_TEXT = NULL;
    }
}

static GMarkupParser parser = {
    start_element,
    end_element,
    text,
    NULL,
    NULL
};

Comic *
tbo_comic_load (char *filename)
{
    char *text;
    gsize length;
    GMarkupParseContext *context = g_markup_parse_context_new (
            &parser,
            0,
            NULL,
            NULL);

    char base_name[255];
    get_base_name (filename, base_name, 255);
    TITLE = g_strdup(base_name);

    if (g_file_get_contents (filename, &text, &length, NULL) == FALSE) {
        GtkWidget *dialog = gtk_message_dialog_new (NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_CLOSE,
                _("Couldn't load file"));
        gtk_dialog_run (GTK_DIALOG (dialog));
        return NULL;
    }

    if (g_markup_parse_context_parse (context, text, length, NULL) == FALSE) {
        GtkWidget *dialog = gtk_message_dialog_new (NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_CLOSE,
                _("Couldn't parse file"));
        gtk_dialog_run (GTK_DIALOG (dialog));
        return NULL;
    }

    g_free(text);
    g_markup_parse_context_free (context);

    return COMIC;
}
