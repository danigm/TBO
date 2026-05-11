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


#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "doodle-treeview.h"
#include "dnd.h"
#include "tbo-files.h"
#include "tbo-ui-utils.h"
#include "tbo-widget.h"

typedef struct
{
    TboWindow *tbo;
    GString *path;
    gboolean top_level;
    gboolean bubble_mode;
} DoodleExpanderData;

typedef struct
{
    TboWindow *tbo;
    GtkWidget *search_entry;
    GtkWidget *content_box;
    gboolean bubble_mode;
} DoodleBrowserState;

static GHashTable *THUMB_CACHE = NULL;

#define TBO_BODY_THUMB_MIN_DIM 80
#define TBO_BODY_THUMB_MAX_DIM 96

static GdkPixbuf *get_thumbnail_pixbuf (const gchar *path, const gchar *relative_path);
static gint compare_gstrings (gconstpointer a, gconstpointer b);
static void sort_gstring_array (GArray *arr);
static gchar *normalize_search_text (const gchar *text);
static gboolean search_matches_text (const gchar *text, const gchar *query);
static gchar *humanize_label (const gchar *path);
static gchar *format_expander_label (const gchar *path, gint count, gboolean top_level);
static GtkWidget *create_dir_expander (const gchar *path, gint count, gboolean top_level);
static gint count_matching_assets_in_dir (const gchar *dir, const gchar *query);
static void asset_button_clicked_cb (GtkButton *button, gpointer user_data);
static GtkWidget *build_image_grid_internal (TboWindow *tbo, gchar *dir, const gchar *query, gboolean allow_empty);
static void free_expander_data (gpointer data, GClosure *closure);
static void on_expand_cb (GtkExpander *expander, GParamSpec *pspec, DoodleExpanderData *data);
static GtkWidget *doodle_create_no_results_label (void);
static gboolean populate_filtered_dir (TboWindow *tbo, const gchar *dir, GtkWidget *box, const gchar *query, gboolean top_level);
static void rebuild_browser_content (DoodleBrowserState *state);
static void search_changed_cb (GtkEditable *editable, gpointer user_data);

void
doodle_free_all (void)
{
    if (THUMB_CACHE != NULL)
        g_hash_table_remove_all (THUMB_CACHE);
}

static GdkPixbuf *
get_thumbnail_pixbuf (const gchar *path, const gchar *relative_path)
{
    GdkPixbuf *pixbuf;
    gint width;
    gint height;
    gint max_dim;
    gdouble scale;
    gboolean is_body;

    if (THUMB_CACHE == NULL)
        THUMB_CACHE = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

    pixbuf = g_hash_table_lookup (THUMB_CACHE, path);
    if (pixbuf != NULL)
        return g_object_ref (pixbuf);

    pixbuf = gdk_pixbuf_new_from_file (path, NULL);
    if (pixbuf == NULL)
        return NULL;

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    max_dim = MAX (width, height);
    is_body = g_strrstr (relative_path, "/body/") != NULL || g_str_has_prefix (relative_path, "body/");

    if (is_body && max_dim < TBO_BODY_THUMB_MIN_DIM)
        scale = (gdouble) TBO_BODY_THUMB_MIN_DIM / max_dim;
    else if (is_body && max_dim > TBO_BODY_THUMB_MAX_DIM)
        scale = (gdouble) TBO_BODY_THUMB_MAX_DIM / max_dim;
    else
        scale = 1.0;

    if (scale != 1.0)
    {
        GdkPixbuf *scaled = gdk_pixbuf_scale_simple (pixbuf,
                                                     MAX (1, round (width * scale)),
                                                     MAX (1, round (height * scale)),
                                                     GDK_INTERP_BILINEAR);
        g_object_unref (pixbuf);
        pixbuf = scaled;
        if (pixbuf == NULL)
            return NULL;
    }

    g_hash_table_insert (THUMB_CACHE, g_strdup (path), g_object_ref (pixbuf));
    return pixbuf;
}

void
free_gstring_array (GArray *arr)
{
    int i;

    if (arr == NULL)
        return;

    for (i = 0; i < (int) arr->len; i++)
    {
        GString *mystr = g_array_index (arr, GString *, i);

        g_string_free (mystr, TRUE);
    }

    g_array_free (arr, TRUE);
}

static gint
compare_gstrings (gconstpointer a, gconstpointer b)
{
    const GString *sa = *(const GString * const *) a;
    const GString *sb = *(const GString * const *) b;

    return g_utf8_collate (sa->str, sb->str);
}

static void
sort_gstring_array (GArray *arr)
{
    if (arr != NULL && arr->len > 1)
        g_array_sort (arr, compare_gstrings);
}

GArray *
get_files (gchar *base_dir, gboolean isdir, gboolean bubble_mode)
{
    GError *error = NULL;
    const gchar *filename;
    GDir *dir;
    struct stat filestat;
    int st;
    GArray *array = g_array_new (FALSE, FALSE, sizeof (GString *));

    st = stat (base_dir, &filestat);
    if (st)
    {
        g_array_free (array, TRUE);
        return NULL;
    }

    dir = g_dir_open (base_dir, 0, &error);
    if (dir == NULL)
    {
        if (error != NULL)
            g_error_free (error);
        g_array_free (array, TRUE);
        return NULL;
    }

    while ((filename = g_dir_read_name (dir)) != NULL)
    {
        gchar *complete_dir = g_build_filename (base_dir, filename, NULL);

        st = stat (complete_dir, &filestat);
        if (st)
        {
            g_free (complete_dir);
            continue;
        }

        if (isdir && bubble_mode && strcmp (filename, "bubble") != 0)
        {
            g_free (complete_dir);
            continue;
        }
        if (!strcmp (filename, "bubble") && !bubble_mode)
        {
            g_free (complete_dir);
            continue;
        }

        if (isdir && S_ISDIR (filestat.st_mode))
        {
            GString *dirname_to_append = g_string_new (complete_dir);
            g_array_append_val (array, dirname_to_append);
        }
        else if (!isdir && !S_ISDIR (filestat.st_mode))
        {
            if (!tbo_files_is_supported_asset_file (complete_dir))
            {
                g_free (complete_dir);
                continue;
            }

            GString *filename_to_append = g_string_new (complete_dir);
            g_array_append_val (array, filename_to_append);
        }

        g_free (complete_dir);
    }

    g_dir_close (dir);
    sort_gstring_array (array);
    return array;
}

static gchar *
normalize_search_text (const gchar *text)
{
    gchar *normalized;
    gint i;

    if (text == NULL)
        return g_strdup ("");

    normalized = g_utf8_casefold (text, -1);
    for (i = 0; normalized[i] != '\0'; i++)
    {
        if (normalized[i] == '-' || normalized[i] == '_' || normalized[i] == '/' || normalized[i] == '.')
            normalized[i] = ' ';
    }

    return normalized;
}

static gboolean
search_matches_text (const gchar *text, const gchar *query)
{
    gchar *normalized_text;
    gchar *normalized_query;
    gboolean matches;

    if (query == NULL || *query == '\0')
        return TRUE;

    normalized_text = normalize_search_text (text);
    normalized_query = normalize_search_text (query);
    matches = g_strstr_len (normalized_text, -1, normalized_query) != NULL;
    g_free (normalized_text);
    g_free (normalized_query);
    return matches;
}

static gchar *
humanize_label (const gchar *path)
{
    gchar *basename = g_path_get_basename (path);
    gint i;

    for (i = 0; basename[i] != '\0'; i++)
    {
        if (basename[i] == '-' || basename[i] == '_')
            basename[i] = ' ';
    }
    if (basename[0] != '\0')
        basename[0] = g_ascii_toupper (basename[0]);

    return basename;
}

static gchar *
format_expander_label (const gchar *path, gint count, gboolean top_level)
{
    gchar *title = humanize_label (path);
    gchar *label;

    if (top_level)
        label = g_strdup_printf ("<span weight=\"bold\">%s</span> <span alpha=\"70%%\">(%d)</span>", title, count);
    else
        label = g_strdup_printf ("%s (%d)", title, count);

    g_free (title);
    return label;
}

static GtkWidget *
create_dir_expander (const gchar *path, gint count, gboolean top_level)
{
    GtkWidget *expander;
    gchar *label = format_expander_label (path, count, top_level);

    expander = gtk_expander_new (label);
    gtk_widget_add_css_class (expander, top_level ? "tbo-sidebar-group" : "tbo-sidebar-subgroup");
    if (top_level)
        gtk_expander_set_use_markup (GTK_EXPANDER (expander), TRUE);
    g_free (label);
    return expander;
}

static gint
count_matching_assets_in_dir (const gchar *dir, const gchar *query)
{
    GArray *files;
    GArray *subdirs;
    gint count = 0;
    gint i;

    files = get_files ((gchar *) dir, FALSE, FALSE);
    if (files != NULL)
    {
        for (i = 0; i < (int) files->len; i++)
        {
            GString *file = g_array_index (files, GString *, i);
            const gchar *relative_path;
            gint prefix_len = tbo_files_prefix_len (file->str);

            relative_path = prefix_len > 0 ? file->str + prefix_len : file->str;
            if (search_matches_text (relative_path, query))
                count++;
        }
        free_gstring_array (files);
    }

    subdirs = get_files ((gchar *) dir, TRUE, FALSE);
    if (subdirs != NULL)
    {
        for (i = 0; i < (int) subdirs->len; i++)
        {
            GString *subdir = g_array_index (subdirs, GString *, i);

            count += count_matching_assets_in_dir (subdir->str, query);
        }
        free_gstring_array (subdirs);
    }

    return count;
}

static void
asset_button_clicked_cb (GtkButton *button, gpointer user_data)
{
    TboWindow *tbo = user_data;
    const gchar *asset_path = g_object_get_data (G_OBJECT (button), "tbo-asset-relative-path");

    if (tbo == NULL)
        return;

    if (asset_path == NULL)
        asset_path = g_object_get_data (G_OBJECT (button), "tbo-asset-full-path");

    if (tbo_dnd_insert_asset_centered (tbo, asset_path) != NULL)
        gtk_widget_grab_focus (tbo->drawing);
}

static GtkWidget *
build_image_grid_internal (TboWindow *tbo, gchar *dir, const gchar *query, gboolean allow_empty)
{
    GArray *arr;
    GtkWidget *grid;
    gint visible = 0;
    gint i;

    arr = get_files (dir, FALSE, FALSE);
    grid = gtk_grid_new ();
    gtk_widget_add_css_class (grid, "tbo-asset-grid");
    gtk_grid_set_row_spacing (GTK_GRID (grid), 8);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 8);

    if (arr == NULL)
    {
        if (allow_empty)
        {
            tbo_widget_show_all (grid);
            return grid;
        }
        return NULL;
    }

    for (i = 0; i < (int) arr->len; i++)
    {
        GString *mystr = g_array_index (arr, GString *, i);
        const gchar *relative_path;
        gint prefix_len = tbo_files_prefix_len (mystr->str);
        GdkPixbuf *pixbuf;
        GtkWidget *image;
        GtkWidget *button;
        gint thumb_width;
        gint thumb_height;
        gint left;
        gint top;

        relative_path = prefix_len > 0 ? mystr->str + prefix_len : mystr->str;
        if (!search_matches_text (relative_path, query))
            continue;

        pixbuf = get_thumbnail_pixbuf (mystr->str, relative_path);
        if (pixbuf == NULL)
            continue;

        top = visible / 2;
        left = visible % 2;
        visible++;

        thumb_width = gdk_pixbuf_get_width (pixbuf);
        thumb_height = gdk_pixbuf_get_height (pixbuf);
        image = tbo_picture_new_for_pixbuf (pixbuf);
        gtk_picture_set_can_shrink (GTK_PICTURE (image), TRUE);
        tbo_picture_set_contain (GTK_PICTURE (image));
        gtk_widget_set_size_request (image, thumb_width, thumb_height);

        button = gtk_button_new ();
        gtk_button_set_has_frame (GTK_BUTTON (button), FALSE);
        gtk_widget_set_can_focus (button, TRUE);
        gtk_widget_set_size_request (button, thumb_width + 12, thumb_height + 12);
        gtk_widget_add_css_class (button, "tbo-asset-button");
        gtk_widget_set_tooltip_text (button, relative_path);
        tbo_dnd_setup_asset_source (button, mystr->str, relative_path);
        g_signal_connect (button, "clicked", G_CALLBACK (asset_button_clicked_cb), tbo);

        tbo_widget_add_child (button, image);
        gtk_grid_attach (GTK_GRID (grid), button, left, top, 1, 1);
        g_object_unref (pixbuf);
    }

    free_gstring_array (arr);

    if (!allow_empty && visible == 0)
        return NULL;

    tbo_widget_show_all (grid);
    return grid;
}

GtkWidget *
doodle_add_images (gchar *dir)
{
    return build_image_grid_internal (NULL, dir, NULL, TRUE);
}

void
doodle_add_dir_images (gchar *dir, GtkWidget *box)
{
    GtkWidget *expander = create_dir_expander (dir, count_matching_assets_in_dir (dir, NULL), FALSE);
    GtkWidget *grid = build_image_grid_internal (NULL, dir, NULL, TRUE);

    tbo_widget_add_child (expander, grid);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
    tbo_widget_add_child (box, expander);
}

static void
free_expander_data (gpointer data, GClosure *closure)
{
    DoodleExpanderData *expander_data = data;

    (void) closure;

    if (expander_data != NULL)
    {
        if (expander_data->path != NULL)
            g_string_free (expander_data->path, TRUE);
        g_free (expander_data);
    }
}

static void
on_expand_cb (GtkExpander *expander, GParamSpec *pspec, DoodleExpanderData *data)
{
    GtkWidget *vbox = gtk_expander_get_child (expander);
    gint num_children;
    GArray *subdirs;
    GtkWidget *grid;
    gint i;

    (void) pspec;

    if (vbox == NULL || !gtk_expander_get_expanded (expander) || data == NULL)
        return;

    num_children = tbo_widget_get_child_count (vbox);
    if (num_children > 0)
        return;

    subdirs = get_files (data->path->str, TRUE, FALSE);
    if (subdirs != NULL)
    {
        for (i = 0; i < (int) subdirs->len; i++)
        {
            GString *subdir = g_array_index (subdirs, GString *, i);
            gint count = count_matching_assets_in_dir (subdir->str, NULL);
            GtkWidget *child_expander;
            GtkWidget *child_box;
            DoodleExpanderData *child_data;

            if (count == 0)
                continue;

            child_expander = create_dir_expander (subdir->str, count, FALSE);
            child_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
            tbo_widget_add_child (child_expander, child_box);
            tbo_box_pack_start (vbox, child_expander, FALSE, FALSE, 5);

            child_data = g_new0 (DoodleExpanderData, 1);
            child_data->tbo = data->tbo;
            child_data->path = g_string_new (subdir->str);
            child_data->top_level = FALSE;
            child_data->bubble_mode = data->bubble_mode;
            g_signal_connect_data (child_expander,
                                   "notify::expanded",
                                   G_CALLBACK (on_expand_cb),
                                   child_data,
                                   free_expander_data,
                                   0);

            if (data->bubble_mode)
            {
                gtk_expander_set_expanded (GTK_EXPANDER (child_expander), TRUE);
                on_expand_cb (GTK_EXPANDER (child_expander), NULL, child_data);
            }
        }

        free_gstring_array (subdirs);
    }

    grid = build_image_grid_internal (data->tbo, data->path->str, NULL, FALSE);
    if (grid != NULL)
        tbo_widget_add_child (vbox, grid);

    tbo_widget_show_all (vbox);
}

static GtkWidget *
doodle_create_no_results_label (void)
{
    GtkWidget *label = gtk_label_new (_("No assets match this search."));

    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_widget_add_css_class (label, "dim-label");
    return label;
}

static gboolean
populate_filtered_dir (TboWindow *tbo, const gchar *dir, GtkWidget *box, const gchar *query, gboolean top_level)
{
    GArray *subdirs;
    GtkWidget *expander;
    GtkWidget *content;
    GtkWidget *grid;
    gint i;
    gint count = count_matching_assets_in_dir (dir, query);

    if (count == 0)
        return FALSE;

    expander = create_dir_expander (dir, count, top_level);
    content = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    tbo_widget_add_child (expander, content);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);

    subdirs = get_files ((gchar *) dir, TRUE, FALSE);
    if (subdirs != NULL)
    {
        for (i = 0; i < (int) subdirs->len; i++)
        {
            GString *subdir = g_array_index (subdirs, GString *, i);

            populate_filtered_dir (tbo, subdir->str, content, query, FALSE);
        }
        free_gstring_array (subdirs);
    }

    grid = build_image_grid_internal (tbo, (gchar *) dir, query, FALSE);
    if (grid != NULL)
        tbo_widget_add_child (content, grid);

    tbo_box_pack_start (box, expander, FALSE, FALSE, 5);
    return TRUE;
}

static void
rebuild_browser_content (DoodleBrowserState *state)
{
    gchar **possible_dirs;
    const gchar *query;
    gboolean added_any = FALSE;
    gint k;

    if (state == NULL)
        return;

    tbo_widget_destroy_all_children (state->content_box);
    query = gtk_editable_get_text (GTK_EDITABLE (state->search_entry));
    possible_dirs = tbo_files_get_dirs ();

    for (k = 0; possible_dirs[k] != NULL; k++)
    {
        GArray *arr = get_files (possible_dirs[k], TRUE, state->bubble_mode);
        gint i;

        if (arr == NULL)
            continue;

        for (i = 0; i < (int) arr->len; i++)
        {
            GString *dir = g_array_index (arr, GString *, i);

            if (query != NULL && *query != '\0')
            {
                if (populate_filtered_dir (state->tbo, dir->str, state->content_box, query, TRUE))
                    added_any = TRUE;
            }
            else
            {
                gint count = count_matching_assets_in_dir (dir->str, NULL);
                GtkWidget *expander;
                GtkWidget *vbox;
                DoodleExpanderData *expander_data;

                if (count == 0)
                    continue;

                expander = create_dir_expander (dir->str, count, TRUE);
                vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
                tbo_widget_add_child (expander, vbox);
                tbo_box_pack_start (state->content_box, expander, FALSE, FALSE, 5);

                expander_data = g_new0 (DoodleExpanderData, 1);
                expander_data->tbo = state->tbo;
                expander_data->path = g_string_new (dir->str);
                expander_data->top_level = TRUE;
                expander_data->bubble_mode = state->bubble_mode;
                g_signal_connect_data (expander,
                                       "notify::expanded",
                                       G_CALLBACK (on_expand_cb),
                                       expander_data,
                                       free_expander_data,
                                       0);

                if (state->bubble_mode)
                {
                    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
                    on_expand_cb (GTK_EXPANDER (expander), NULL, expander_data);
                }

                added_any = TRUE;
            }
        }

        free_gstring_array (arr);
    }

    if (!added_any)
        tbo_widget_add_child (state->content_box, doodle_create_no_results_label ());

    tbo_files_free (possible_dirs);
    tbo_widget_show_all (state->content_box);
}

static void
search_changed_cb (GtkEditable *editable, gpointer user_data)
{
    (void) editable;
    rebuild_browser_content (user_data);
}

GtkWidget *
doodle_setup_tree (TboWindow *tbo, gboolean bubble_mode)
{
    GtkWidget *root;
    GtkWidget *search_entry;
    GtkWidget *content;
    DoodleBrowserState *state;

    root = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
    search_entry = gtk_search_entry_new ();
    gtk_widget_add_css_class (search_entry, "tbo-sidebar-search");
    gtk_widget_set_margin_start (search_entry, 4);
    gtk_widget_set_margin_end (search_entry, 4);
    gtk_widget_set_margin_top (search_entry, 4);
    gtk_widget_set_margin_bottom (search_entry, 4);
#if GTK_CHECK_VERSION(4, 10, 0)
    gtk_search_entry_set_placeholder_text (GTK_SEARCH_ENTRY (search_entry),
                                           bubble_mode ? _("Search Bubbles") : _("Search Assets"));
#else
    if (g_object_class_find_property (G_OBJECT_GET_CLASS (search_entry), "placeholder-text") != NULL)
        g_object_set (search_entry,
                      "placeholder-text",
                      bubble_mode ? _("Search Bubbles") : _("Search Assets"),
                      NULL);
#endif

    content = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    tbo_widget_add_child (root, search_entry);
    tbo_widget_add_child (root, content);

    state = g_new0 (DoodleBrowserState, 1);
    state->tbo = tbo;
    state->search_entry = search_entry;
    state->content_box = content;
    state->bubble_mode = bubble_mode;
    g_object_set_data_full (G_OBJECT (root), "tbo-browser-state", state, g_free);
    g_signal_connect (search_entry, "search-changed", G_CALLBACK (search_changed_cb), state);

    rebuild_browser_content (state);
    return root;
}
