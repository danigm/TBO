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

#include "tbo-clip-lib.h"

#include <string.h>

#define WARN_ERROR \
    if (error) { \
        g_warning ("%s", error->message); \
        g_error_free (error); \
    }

#define WARN_ERROR_RETURN_NULL \
    if (error) { \
        g_warning ("%s", error->message); \
        g_error_free (error); \
        return NULL; \
    }

G_DEFINE_TYPE (TboClipLib, tbo_clip_lib, G_TYPE_OBJECT);

static void tbo_clip_lib_dispose (GObject *object);
static void tbo_clip_lib_finalize (GObject *object);

static void
tbo_clip_lib_dispose (GObject *object)
{
    printf ("dispose\n");
    TboClipLib *clip_lib = TBO_CLIP_LIB (object);

    if (clip_lib->icon) {
        gdk_pixbuf_unref(clip_lib->icon);
        clip_lib->icon = NULL;
    }

    G_OBJECT_CLASS (tbo_clip_lib_parent_class)->dispose (object);
}

static void
tbo_clip_lib_finalize (GObject *object)
{
    printf ("finalize\n");
    TboClipLib *clip_lib = TBO_CLIP_LIB (object);

    if (clip_lib->path) {
        g_free (clip_lib->path);
        clip_lib->path = 0;
    }

    if (clip_lib->name) {
        g_free (clip_lib->name);
        clip_lib->name = 0;
    }

    if (clip_lib->desc) {
        g_free (clip_lib->desc);
        clip_lib->desc = 0;
    }

    if (clip_lib->author) {
        g_free (clip_lib->author);
        clip_lib->author = 0;
    }

    if (clip_lib->license) {
        g_free (clip_lib->license);
        clip_lib->license = 0;
    }

    if (clip_lib->version) {
        g_free (clip_lib->version);
        clip_lib->version = 0;
    }

    if (clip_lib->link) {
        g_free (clip_lib->link);
        clip_lib->link = 0;
    }

    G_OBJECT_CLASS (tbo_clip_lib_parent_class)->finalize (object);
}

GObject *
tbo_clip_lib_new (const gchar *path)
{
    GObject *obj;
    TboClipLib *clip_lib;
    GError *error = NULL;
    gchar thumb_path[255] = {0}, conf_path[255] = {0};
    GKeyFile *conffile;
    size_t strsize;

    obj = g_object_new (TBO_TYPE_CLIP_LIB, NULL);
    clip_lib = TBO_CLIP_LIB (obj);

    clip_lib->path = g_strdup (path);
    clip_lib->name = NULL;
    clip_lib->icon = NULL;

    strsize = sizeof (char) * (strlen (clip_lib->path) + strlen ("thumb.png") + 2);
    snprintf (thumb_path, strsize, "%s/%s", clip_lib->path, "thumb.png");
    clip_lib->icon = gdk_pixbuf_new_from_file (thumb_path, &error);
    WARN_ERROR_RETURN_NULL

    strsize = sizeof (char) * (strlen (clip_lib->path) + strlen ("tbolib.conf") + 2);
    snprintf (conf_path, strsize, "%s/%s", clip_lib->path, "tbolib.conf");

    conffile = g_key_file_new ();

    g_key_file_load_from_file (conffile, conf_path, G_KEY_FILE_NONE, &error);
    WARN_ERROR_RETURN_NULL
    clip_lib->name = g_key_file_get_value (conffile, "description", "name", &error);
    WARN_ERROR_RETURN_NULL
    clip_lib->desc = g_key_file_get_value (conffile, "description", "desc", &error);
    WARN_ERROR_RETURN_NULL
    clip_lib->author = g_key_file_get_value (conffile, "description", "author", &error);
    WARN_ERROR_RETURN_NULL
    clip_lib->license = g_key_file_get_value (conffile, "description", "license", &error);
    WARN_ERROR_RETURN_NULL
    clip_lib->version = g_key_file_get_value (conffile, "description", "version", &error);
    WARN_ERROR_RETURN_NULL
    clip_lib->link = g_key_file_get_value (conffile, "description", "link", &error);
    WARN_ERROR_RETURN_NULL

    return obj;
}

static void
tbo_clip_lib_init (TboClipLib *self)
{
}

static void
tbo_clip_lib_class_init (TboClipLibClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = tbo_clip_lib_dispose;
    object_class->finalize = tbo_clip_lib_finalize;
}
