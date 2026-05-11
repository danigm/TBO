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


#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include "tbo-types.h"
#include "tbo-window.h"
#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-group.h"
#include "tbo-object-pixmap.h"
#include "tbo-object-svg.h"
#include "tbo-object-text.h"
#include "ui-menu.h"
#include "tbo-toolbar.h"
#include "tbo-drawing.h"
#include "tbo-tool-frame.h"
#include "tbo-tool-selector.h"
#include "tbo-tool-text.h"
#include "tbo-tooltip.h"
#include "tbo-utils.h"
#include "tbo-widget.h"
#include "comic-saveas-dialog.h"

static gboolean on_key_cb (GtkEventControllerKey *controller,
                           guint keyval,
                           guint keycode,
                           GdkModifierType state,
                           TboWindow *tbo);

#define TBO_RECENT_PROJECT_LIMIT 5

static gchar *get_state_file_path (void);
static gchar *get_recovery_dir_path (void);
static gchar *get_recovery_meta_path (const gchar *autosave_file);
static GKeyFile *load_state_key_file (void);
static void save_state_key_file (GKeyFile *kf);
typedef enum
{
    TBO_CONFIRM_CLOSE_CANCEL,
    TBO_CONFIRM_CLOSE_CONTINUE,
    TBO_CONFIRM_CLOSE_DISCARD,
} TboConfirmCloseResult;
static TboConfirmCloseResult confirm_close (TboWindow *tbo);
static void update_window_title (TboWindow *tbo);
static void apply_theme_preferences (void);
static TboThemeMode load_theme_mode_preference (void);
static void save_theme_mode_preference (TboThemeMode mode);
static void schedule_autosave (TboWindow *tbo);
static void delete_recovery_files_for_window (TboWindow *tbo);
static void set_window_path (gchar **slot, const gchar *path);

#define TBO_PAGE_WIDGET_KEY "tbo-page"

static gchar *
get_recovery_dir_path (void)
{
    gchar *dir = g_build_filename (g_get_user_config_dir (), "tbo", "recovery", NULL);

    g_mkdir_with_parents (dir, 0755);
    return dir;
}

static gchar *
get_recovery_meta_path (const gchar *autosave_file)
{
    return g_strdup_printf ("%s.ini", autosave_file);
}

static GKeyFile *
load_state_key_file (void)
{
    GKeyFile *kf = g_key_file_new ();
    gchar *state_file = get_state_file_path ();

    g_key_file_load_from_file (kf, state_file, G_KEY_FILE_NONE, NULL);
    g_free (state_file);
    return kf;
}

static void
save_state_key_file (GKeyFile *kf)
{
    gchar *state_file = get_state_file_path ();
    gchar *content;
    gsize len;

    content = g_key_file_to_data (kf, &len, NULL);
    g_file_set_contents (state_file, content, len, NULL);
    g_free (content);
    g_free (state_file);
}

static const gchar *
theme_mode_to_string (TboThemeMode mode)
{
    switch (mode)
    {
        case TBO_THEME_MODE_DARK:
            return "dark";
        case TBO_THEME_MODE_LIGHT:
            return "light";
        case TBO_THEME_MODE_SYSTEM:
        default:
            return "system";
    }
}

static TboThemeMode
theme_mode_from_string (const gchar *mode)
{
    if (g_strcmp0 (mode, "dark") == 0)
        return TBO_THEME_MODE_DARK;
    if (g_strcmp0 (mode, "light") == 0)
        return TBO_THEME_MODE_LIGHT;

    return TBO_THEME_MODE_SYSTEM;
}

static TboThemeMode
load_theme_mode_preference (void)
{
    GKeyFile *kf = load_state_key_file ();
    TboThemeMode mode = TBO_THEME_MODE_SYSTEM;

    if (g_key_file_has_key (kf, "ui", "theme-mode", NULL))
    {
        gchar *mode_name = g_key_file_get_string (kf, "ui", "theme-mode", NULL);

        mode = theme_mode_from_string (mode_name);
        g_free (mode_name);
    }
    else if (g_key_file_has_key (kf, "ui", "light-theme", NULL))
    {
        mode = g_key_file_get_boolean (kf, "ui", "light-theme", NULL) ?
               TBO_THEME_MODE_LIGHT :
               TBO_THEME_MODE_DARK;
    }

    g_key_file_unref (kf);
    return mode;
}

static void
save_theme_mode_preference (TboThemeMode mode)
{
    GKeyFile *kf = load_state_key_file ();

    g_key_file_set_string (kf, "ui", "theme-mode", theme_mode_to_string (mode));
    g_key_file_remove_key (kf, "ui", "light-theme", NULL);
    save_state_key_file (kf);
    g_key_file_unref (kf);
}

static void
update_window_title (TboWindow *tbo)
{
    const gchar *comic_title;
    gchar *window_title;

    if (tbo == NULL || tbo->window == NULL || tbo->comic == NULL)
        return;

    comic_title = tbo_comic_get_title (tbo->comic);
    if (comic_title == NULL || *comic_title == '\0')
        comic_title = _("Untitled");

    if (tbo->dirty)
        window_title = g_strdup_printf ("* %s", comic_title);
    else
        window_title = g_strdup (comic_title);

    gtk_window_set_title (GTK_WINDOW (tbo->window), window_title);
    g_free (window_title);
}

static void
apply_theme_preferences (void)
{
    static gboolean defaults_initialized = FALSE;
    static gchar *system_theme_name = NULL;
    static gboolean system_prefer_dark = FALSE;
    static gboolean has_theme_name = FALSE;
    static gboolean has_prefer_dark = FALSE;
    GtkSettings *settings = gtk_settings_get_default ();
    TboThemeMode mode;

    if (settings == NULL)
        return;

    if (!defaults_initialized)
    {
        has_theme_name = g_object_class_find_property (G_OBJECT_GET_CLASS (settings), "gtk-theme-name") != NULL;
        has_prefer_dark = g_object_class_find_property (G_OBJECT_GET_CLASS (settings), "gtk-application-prefer-dark-theme") != NULL;

        if (has_theme_name)
            g_object_get (settings, "gtk-theme-name", &system_theme_name, NULL);
        if (has_prefer_dark)
            g_object_get (settings, "gtk-application-prefer-dark-theme", &system_prefer_dark, NULL);

        defaults_initialized = TRUE;
    }

    mode = load_theme_mode_preference ();

    if (mode == TBO_THEME_MODE_SYSTEM)
    {
        if (has_theme_name && system_theme_name != NULL)
            g_object_set (settings, "gtk-theme-name", system_theme_name, NULL);
        if (has_prefer_dark)
            g_object_set (settings, "gtk-application-prefer-dark-theme", system_prefer_dark, NULL);
        return;
    }

    if (has_theme_name)
        g_object_set (settings, "gtk-theme-name", "Adwaita", NULL);
    if (has_prefer_dark)
        g_object_set (settings,
                      "gtk-application-prefer-dark-theme",
                      mode == TBO_THEME_MODE_DARK,
                      NULL);
}

static void
remove_recent_project (const gchar *path)
{
    GKeyFile *kf;
    gchar **recent_paths;
    gsize recent_count = 0;
    GPtrArray *filtered;
    gsize i;
    gchar *last_project;

    if (path == NULL || *path == '\0')
        return;

    kf = load_state_key_file ();
    recent_paths = g_key_file_get_string_list (kf, "recent", "files", &recent_count, NULL);
    filtered = g_ptr_array_new_with_free_func (g_free);

    for (i = 0; i < recent_count; i++)
    {
        if (g_strcmp0 (recent_paths[i], path) != 0)
            g_ptr_array_add (filtered, g_strdup (recent_paths[i]));
    }

    if (filtered->len > 0)
    {
        gchar **values = (gchar **) filtered->pdata;

        g_key_file_set_string_list (kf, "recent", "files", (const gchar * const *) values, filtered->len);
    }
    else
    {
        g_key_file_remove_key (kf, "recent", "files", NULL);
    }

    last_project = g_key_file_get_string (kf, "paths", "last_project", NULL);
    if (g_strcmp0 (last_project, path) == 0)
        g_key_file_remove_key (kf, "paths", "last_project", NULL);

    save_state_key_file (kf);
    g_free (last_project);
    g_strfreev (recent_paths);
    g_ptr_array_free (filtered, TRUE);
    g_key_file_unref (kf);
}

void
tbo_window_add_recent_project (const gchar *path)
{
    GKeyFile *kf;
    gchar **recent_paths;
    gsize recent_count = 0;
    GPtrArray *updated;
    gsize i;

    if (path == NULL || *path == '\0')
        return;

    kf = load_state_key_file ();
    recent_paths = g_key_file_get_string_list (kf, "recent", "files", &recent_count, NULL);
    updated = g_ptr_array_new_with_free_func (g_free);
    g_ptr_array_add (updated, g_strdup (path));

    for (i = 0; i < recent_count && updated->len < TBO_RECENT_PROJECT_LIMIT; i++)
    {
        if (g_strcmp0 (recent_paths[i], path) != 0 && recent_paths[i][0] != '\0')
            g_ptr_array_add (updated, g_strdup (recent_paths[i]));
    }

    g_key_file_set_string (kf, "paths", "last_project", path);
    g_key_file_set_string_list (kf,
                                "recent",
                                "files",
                                (const gchar * const *) updated->pdata,
                                updated->len);
    save_state_key_file (kf);

    g_strfreev (recent_paths);
    g_ptr_array_free (updated, TRUE);
    g_key_file_unref (kf);
}

gchar **
tbo_window_get_recent_projects (gsize *n_projects)
{
    GKeyFile *kf = load_state_key_file ();
    gchar **recent_paths;
    gsize recent_count = 0;

    recent_paths = g_key_file_get_string_list (kf, "recent", "files", &recent_count, NULL);
    g_key_file_unref (kf);

    if (n_projects != NULL)
        *n_projects = recent_count;

    return recent_paths;
}

gchar *
tbo_window_get_last_project (void)
{
    GKeyFile *kf = load_state_key_file ();
    gchar *value = g_key_file_get_string (kf, "paths", "last_project", NULL);

    g_key_file_unref (kf);
    return value;
}

void
tbo_window_delete_recovery_file (const gchar *autosave_file)
{
    gchar *meta_file;

    if (autosave_file == NULL || *autosave_file == '\0')
        return;

    g_remove (autosave_file);
    meta_file = get_recovery_meta_path (autosave_file);
    g_remove (meta_file);
    g_free (meta_file);
}

void
tbo_window_clear_persisted_state (void)
{
    gchar *state_file = get_state_file_path ();
    gchar *recovery_dir = get_recovery_dir_path ();
    GDir *dir = g_dir_open (recovery_dir, 0, NULL);
    const gchar *name;

    g_remove (state_file);
    if (dir != NULL)
    {
        while ((name = g_dir_read_name (dir)) != NULL)
        {
            gchar *path = g_build_filename (recovery_dir, name, NULL);

            g_remove (path);
            g_free (path);
        }
        g_dir_close (dir);
    }
    g_rmdir (recovery_dir);
    g_free (recovery_dir);
    g_free (state_file);
}

gchar **
tbo_window_list_recovery_files (gsize *n_files)
{
    gchar *recovery_dir = get_recovery_dir_path ();
    GDir *dir = g_dir_open (recovery_dir, 0, NULL);
    GPtrArray *files = g_ptr_array_new_with_free_func (g_free);
    const gchar *name;

    if (dir != NULL)
    {
        while ((name = g_dir_read_name (dir)) != NULL)
        {
            if (g_str_has_suffix (name, ".tbo"))
                g_ptr_array_add (files, g_build_filename (recovery_dir, name, NULL));
        }
        g_dir_close (dir);
    }

    g_ptr_array_add (files, NULL);
    g_free (recovery_dir);

    if (n_files != NULL)
        *n_files = files->len > 0 ? files->len - 1 : 0;

    return (gchar **) g_ptr_array_free (files, FALSE);
}

static gboolean
autosave_timeout_cb (gpointer user_data)
{
    TboWindow *tbo = user_data;

    tbo->autosave_timeout_id = 0;
    tbo_window_run_autosave (tbo);
    return G_SOURCE_REMOVE;
}

static void
schedule_autosave (TboWindow *tbo)
{
    if (tbo == NULL || tbo->destroying || tbo->autosave_timeout_id != 0)
        return;

    tbo->autosave_timeout_id = g_timeout_add_seconds (1, autosave_timeout_cb, tbo);
}

static void
write_recovery_metadata (TboWindow *tbo)
{
    GKeyFile *kf;
    gchar *meta_file;

    if (tbo == NULL || tbo->autosave_path == NULL)
        return;

    kf = g_key_file_new ();
    meta_file = get_recovery_meta_path (tbo->autosave_path);
    g_key_file_set_string (kf,
                           "recovery",
                           "source_path",
                           tbo->path != NULL ? tbo->path : "");
    g_key_file_set_string (kf,
                           "recovery",
                           "title",
                           tbo_comic_get_title (tbo->comic));

    {
        gchar *content;
        gsize len;

        content = g_key_file_to_data (kf, &len, NULL);
        g_file_set_contents (meta_file, content, len, NULL);
        g_free (content);
    }

    g_free (meta_file);
    g_key_file_unref (kf);
}

static gchar *
load_recovery_source_path (const gchar *autosave_file)
{
    GKeyFile *kf = g_key_file_new ();
    gchar *meta_file = get_recovery_meta_path (autosave_file);
    gchar *source_path = NULL;

    if (g_key_file_load_from_file (kf, meta_file, G_KEY_FILE_NONE, NULL))
    {
        source_path = g_key_file_get_string (kf, "recovery", "source_path", NULL);
        if (source_path != NULL && source_path[0] == '\0')
            g_clear_pointer (&source_path, g_free);
    }

    g_free (meta_file);
    g_key_file_unref (kf);
    return source_path;
}

static void
delete_recovery_files_for_window (TboWindow *tbo)
{
    if (tbo == NULL || tbo->autosave_path == NULL)
        return;

    tbo_window_delete_recovery_file (tbo->autosave_path);
}

static void
add_grid_template (Page *page,
                   gint comic_width,
                   gint comic_height,
                   gint columns,
                   gint rows,
                   gint margin_x,
                   gint margin_y,
                   gint gap_x,
                   gint gap_y)
{
    gint frame_width;
    gint frame_height;
    gint row;
    gint column;

    frame_width = MAX (1, (comic_width - (2 * margin_x) - ((columns - 1) * gap_x)) / columns);
    frame_height = MAX (1, (comic_height - (2 * margin_y) - ((rows - 1) * gap_y)) / rows);

    for (row = 0; row < rows; row++)
    {
        for (column = 0; column < columns; column++)
        {
            gint x = margin_x + (column * (frame_width + gap_x));
            gint y = margin_y + (row * (frame_height + gap_y));

            tbo_page_new_frame (page, x, y, frame_width, frame_height);
        }
    }
}

void
tbo_comic_template_get_default_size (TboComicTemplate template, gint *width, gint *height)
{
    gint default_width = 800;
    gint default_height = 500;

    switch (template)
    {
        case TBO_COMIC_TEMPLATE_STRIP:
            default_width = 1800;
            default_height = 600;
            break;
        case TBO_COMIC_TEMPLATE_A4:
            default_width = 1240;
            default_height = 1754;
            break;
        case TBO_COMIC_TEMPLATE_STORYBOARD:
            default_width = 1600;
            default_height = 900;
            break;
        case TBO_COMIC_TEMPLATE_EMPTY:
        case TBO_COMIC_TEMPLATE_N_TEMPLATES:
        default:
            break;
    }

    if (width != NULL)
        *width = default_width;
    if (height != NULL)
        *height = default_height;
}

void
tbo_window_apply_comic_template (TboWindow *tbo, TboComicTemplate template)
{
    Comic *comic;
    Page *page;
    gint comic_width;
    gint comic_height;

    if (tbo == NULL || tbo->comic == NULL)
        return;

    comic = tbo->comic;
    page = tbo_comic_get_current_page (comic);
    if (page == NULL)
        page = tbo_comic_new_page (comic);

    while (tbo_page_len (page) > 0)
        tbo_page_del_frame_by_index (page, 0);

    comic_width = tbo_comic_get_width (comic);
    comic_height = tbo_comic_get_height (comic);

    switch (template)
    {
        case TBO_COMIC_TEMPLATE_STRIP:
            tbo_comic_set_paper (comic, TBO_COMIC_PAPER_NONE);
            add_grid_template (page,
                               comic_width,
                               comic_height,
                               3,
                               1,
                               MAX (20, comic_width / 30),
                               MAX (20, comic_height / 12),
                               MAX (16, comic_width / 45),
                               0);
            break;
        case TBO_COMIC_TEMPLATE_A4:
            tbo_comic_set_paper (comic, TBO_COMIC_PAPER_A4);
            add_grid_template (page,
                               comic_width,
                               comic_height,
                               2,
                               3,
                               MAX (24, comic_width / 16),
                               MAX (24, comic_height / 24),
                               MAX (16, comic_width / 45),
                               MAX (16, comic_height / 45));
            break;
        case TBO_COMIC_TEMPLATE_STORYBOARD:
            tbo_comic_set_paper (comic, TBO_COMIC_PAPER_NONE);
            add_grid_template (page,
                               comic_width,
                               comic_height,
                               2,
                               2,
                               MAX (24, comic_width / 20),
                               MAX (24, comic_height / 12),
                               MAX (16, comic_width / 35),
                               MAX (16, comic_height / 20));
            break;
        case TBO_COMIC_TEMPLATE_EMPTY:
        case TBO_COMIC_TEMPLATE_N_TEMPLATES:
        default:
            tbo_comic_set_paper (comic, TBO_COMIC_PAPER_NONE);
            break;
    }

    gtk_widget_queue_draw (tbo->drawing);
    tbo_window_refresh_status (tbo);
}

static void
setup_darea_controllers (GtkWidget *darea, TboWindow *tbo)
{
    GtkEventController *key;

    tbo_drawing_init_dnd (TBO_DRAWING (darea), tbo);

    key = gtk_event_controller_key_new ();
    g_signal_connect (key, "key-pressed", G_CALLBACK (on_key_cb), tbo);
    gtk_widget_add_controller (darea, key);
}

static void
detach_document_state (TboWindow *tbo)
{
    if (tbo == NULL)
        return;

    if (tbo->toolbar != NULL && tbo->toolbar->tools != NULL)
    {
        tbo_tool_selector_reset_state (TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]));
        tbo_tool_frame_reset_state (TBO_TOOL_FRAME (tbo->toolbar->tools[TBO_TOOLBAR_FRAME]));
        tbo_tool_text_reset_state (TBO_TOOL_TEXT (tbo->toolbar->tools[TBO_TOOLBAR_TEXT]));
        tbo->toolbar->selected_tool = NULL;
    }

    tbo_undo_stack_clear (tbo->undo_stack);
    tbo->undo_stack->current_state_id = 0;
    tbo->undo_stack->next_state_id = 1;
    tbo_tooltip_reset (tbo);
}

static void
apply_window_icon (GtkWidget *window)
{
    gtk_window_set_default_icon_name ("tbo");
    gtk_window_set_icon_name (GTK_WINDOW (window), "tbo");
}

static GtkWidget *
get_page_widget (TboWindow *tbo, gint nth)
{
    return gtk_notebook_get_nth_page (GTK_NOTEBOOK (tbo->notebook), nth);
}

static Page *
get_page_widget_page (GtkWidget *page_widget)
{
    return page_widget != NULL ? g_object_get_data (G_OBJECT (page_widget), TBO_PAGE_WIDGET_KEY) : NULL;
}

static gint
find_page_widget_index (TboWindow *tbo, Page *page)
{
    gint i;
    gint count;

    if (tbo == NULL || page == NULL)
        return -1;

    count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (tbo->notebook));
    for (i = 0; i < count; i++)
    {
        if (get_page_widget_page (get_page_widget (tbo, i)) == page)
            return i;
    }

    return -1;
}

static GtkWidget *
create_page_tab_label (gint nth)
{
    gchar *text = g_strdup_printf (_("Page %d"), nth + 1);
    GtkWidget *label = gtk_label_new (text);

    g_free (text);
    return label;
}

static void
refresh_page_tab_labels (TboWindow *tbo)
{
    gint i;
    gint count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (tbo->notebook));

    for (i = 0; i < count; i++)
    {
        GtkWidget *page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tbo->notebook), i);
        GtkWidget *label = create_page_tab_label (i);

        gtk_notebook_set_tab_label (GTK_NOTEBOOK (tbo->notebook), page, label);
    }
}

static void
sync_page_widgets_with_comic (TboWindow *tbo)
{
    gint widget_count;
    gint comic_count;
    gint i;

    if (tbo == NULL)
        return;

    widget_count = tbo_window_get_page_count (tbo);
    comic_count = tbo_comic_len (tbo->comic);

    while (widget_count > comic_count)
    {
        tbo_window_remove_page_widget (tbo, widget_count - 1);
        widget_count--;
    }

    for (i = 0; i < comic_count; i++)
    {
        Page *page = g_list_nth_data (tbo_comic_get_pages (tbo->comic), i);
        GtkWidget *widget = i < widget_count ? get_page_widget (tbo, i) : NULL;

        if (widget == NULL)
        {
            tbo_window_insert_page_widget (tbo, create_darea (tbo), page, i);
            widget_count++;
            continue;
        }

        if (get_page_widget_page (widget) != page)
        {
            gint current_index = find_page_widget_index (tbo, page);

            if (current_index >= 0)
            {
                tbo->syncing_page_reorder = TRUE;
                gtk_notebook_reorder_child (GTK_NOTEBOOK (tbo->notebook), get_page_widget (tbo, current_index), i);
                tbo->syncing_page_reorder = FALSE;
            }
            else
            {
                tbo_window_insert_page_widget (tbo, create_darea (tbo), page, i);
                widget_count++;
            }
        }
    }

    refresh_page_tab_labels (tbo);
}

static gboolean
notebook_switch_page_cb (GtkNotebook *notebook,
                         GtkWidget   *page,
                         guint        page_num,
                         TboWindow   *tbo)
{
    if (tbo == NULL || tbo->destroying)
        return FALSE;

    tbo_comic_set_current_page_nth (tbo->comic, page_num);
    tbo_window_set_current_tab_page (tbo, FALSE);
    tbo_toolbar_update (tbo->toolbar);
    tbo_window_refresh_status (tbo);
    tbo_drawing_adjust_scroll (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static void
notebook_page_reordered_cb (GtkNotebook *notebook,
                            GtkWidget   *child,
                            guint        page_num,
                            TboWindow   *tbo)
{
    Page *page;
    gint old_index;

    (void) notebook;

    if (tbo == NULL || tbo->destroying || tbo->syncing_page_reorder)
        return;

    page = get_page_widget_page (child);
    if (page == NULL)
        return;

    old_index = tbo_comic_page_nth (tbo->comic, page);
    if (old_index < 0 || old_index == (gint) page_num)
        return;

    tbo_comic_reorder_page (tbo->comic, page, page_num);
    tbo_undo_stack_insert (tbo->undo_stack, tbo_action_page_reorder_new (tbo->comic, page, old_index, page_num));
    tbo_window_mark_dirty (tbo);
    refresh_page_tab_labels (tbo);
    tbo_window_set_current_tab_page (tbo, FALSE);
    tbo_window_refresh_status (tbo);
}

static void
destroy_cb (GtkWidget *widget, TboWindow *tbo)
{
    tbo_window_free (tbo);
}

static void
set_window_path (gchar **slot, const gchar *path)
{
    g_free (*slot);
    *slot = path != NULL ? g_strdup (path) : NULL;
}

static gchar *
get_state_file_path (void)
{
    gchar *dir = g_build_filename (g_get_user_config_dir (), "tbo", NULL);
    gchar *path;

    g_mkdir_with_parents (dir, 0755);
    path = g_build_filename (dir, "state.ini", NULL);
    g_free (dir);
    return path;
}

static gchar *
load_persisted_path (const gchar *key)
{
    GKeyFile *kf = load_state_key_file ();
    gchar *value = NULL;

    if (g_key_file_has_group (kf, "paths"))
        value = g_key_file_get_string (kf, "paths", key, NULL);

    g_key_file_unref (kf);
    return value;
}

static void
store_persisted_path (const gchar *key, const gchar *value)
{
    GKeyFile *kf = load_state_key_file ();

    g_key_file_set_string (kf, "paths", key, value);
    save_state_key_file (kf);
    g_key_file_unref (kf);
}

static gchar *
get_dirname_or_home (const gchar *path)
{
    if (path != NULL && *path != '\0')
        return g_path_get_dirname (path);

    return g_strdup (g_get_home_dir ());
}

gboolean
tbo_window_prepare_for_document_replace (TboWindow *tbo)
{
    return confirm_close (tbo) != TBO_CONFIRM_CLOSE_CANCEL;
}

static TboConfirmCloseResult
confirm_close (TboWindow *tbo)
{
    gint response;
    static const gchar *buttons[] = {
        "_Cancel",
        "_Don't Save",
        "_Save",
        NULL,
    };

    if (!tbo_window_has_unsaved_changes (tbo))
        return TBO_CONFIRM_CLOSE_CONTINUE;

    response = tbo_alert_choose (GTK_WINDOW (tbo->window),
                                 _("Do you want to save your work before closing?"),
                                 _("Unsaved changes will be lost if you close this window."),
                                 buttons,
                                 0,
                                 2);

    if (response == 2)
        return tbo_comic_save_dialog (NULL, tbo) ? TBO_CONFIRM_CLOSE_CONTINUE : TBO_CONFIRM_CLOSE_CANCEL;

    if (response == 1)
        return TBO_CONFIRM_CLOSE_DISCARD;

    return TBO_CONFIRM_CLOSE_CANCEL;
}

static void
append_status_segment (GString *status, const gchar *segment)
{
    if (segment == NULL || *segment == '\0')
        return;

    if (status->len > 0)
        g_string_append (status, " | ");

    g_string_append (status, segment);
}

static gint
frame_index_for_status (Page *page, Frame *frame)
{
    GList *frames;
    gint index = 1;

    if (page == NULL || frame == NULL)
        return 0;

    for (frames = tbo_page_get_frames (page); frames != NULL; frames = frames->next, index++)
    {
        if (frames->data == frame)
            return index;
    }

    return 0;
}

static const gchar *
object_label_for_status (TboObjectBase *obj)
{
    if (obj == NULL)
        return NULL;
    if (TBO_IS_OBJECT_TEXT (obj))
        return _("Text");
    if (TBO_IS_OBJECT_SVG (obj))
        return _("SVG image");
    if (TBO_IS_OBJECT_PIXMAP (obj))
        return _("Image");
    if (TBO_IS_OBJECT_GROUP (obj))
        return _("Group");

    return _("Object");
}

static void
update_statusbar (TboWindow *tbo)
{
    GString *status;
    Page *page;
    TboToolSelector *selector = NULL;
    Frame *selected_frame = NULL;
    Frame *current_frame;
    TboObjectBase *selected_object = NULL;
    gint frame_index;
    gchar *segment;

    page = tbo_comic_get_current_page (tbo->comic);
    current_frame = tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing));
    if (tbo->toolbar != NULL && tbo->toolbar->tools != NULL)
        selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    if (selector != NULL)
    {
        selected_frame = tbo_tool_selector_get_selected_frame (selector);
        selected_object = tbo_tool_selector_get_selected_obj (selector);
    }

    status = g_string_new (NULL);

    append_status_segment (status, current_frame != NULL ? _("Mode: Frame") : _("Mode: Page"));

    segment = g_strdup_printf (_("Page %d of %d"),
                               tbo_comic_page_position (tbo->comic),
                               tbo_comic_len (tbo->comic));
    append_status_segment (status, segment);
    g_free (segment);

    segment = g_strdup_printf (_("Frames: %d"), page != NULL ? tbo_page_len (page) : 0);
    append_status_segment (status, segment);
    g_free (segment);

    if (current_frame != NULL)
    {
        frame_index = frame_index_for_status (page, current_frame);
        if (frame_index > 0)
            segment = g_strdup_printf (_("Editing frame %d"), frame_index);
        else
            segment = g_strdup (_("Editing frame"));
        append_status_segment (status, segment);
        g_free (segment);

        if (selected_object != NULL)
        {
            segment = g_strdup_printf (_("Object: %s"), object_label_for_status (selected_object));
            append_status_segment (status, segment);
            g_free (segment);
        }

        append_status_segment (status, _("Esc: back to page"));
    }
    else
    {
        if (selected_frame != NULL)
        {
            frame_index = frame_index_for_status (page, selected_frame);
            if (frame_index > 0)
                segment = g_strdup_printf (_("Frame %d selected"), frame_index);
            else
                segment = g_strdup (_("Frame selected"));
            append_status_segment (status, segment);
            g_free (segment);
        }

        append_status_segment (status, _("Enter: frame"));
    }

    gtk_label_set_text (GTK_LABEL (tbo->status), status->str);
    g_string_free (status, TRUE);
}

static void
load_app_css (void)
{
    static gboolean loaded = FALSE;
    GtkCssProvider *provider;
    const gchar *css;

    if (loaded)
        return;

    css =
        "headerbar {"
        "  background: shade(@theme_bg_color, 1.015);"
        "  border-bottom: 1px solid alpha(@theme_fg_color, 0.06);"
        "}"
        "#tbo-toolbar {"
        "  background: shade(@theme_bg_color, 1.03);"
        "  border-bottom: 1px solid alpha(@theme_fg_color, 0.08);"
        "  padding: 4px 0;"
        "}"
        ".tbo-toolbar-section {"
        "  margin-right: 8px;"
        "  padding: 2px;"
        "  border-radius: 12px;"
        "  background: alpha(@theme_fg_color, 0.035);"
        "  border: 1px solid alpha(@theme_fg_color, 0.05);"
        "}"
        ".tbo-toolbar-section button {"
        "  min-width: 38px;"
        "  min-height: 38px;"
        "  padding: 0;"
        "}"
        ".tbo-toolbar-section button:focus-visible {"
        "  box-shadow: inset 0 0 0 2px @accent_bg_color;"
        "}"
        ".tbo-toolbar-icon {"
        "  margin: 0 1px;"
        "}"
        "#tbo-pages > header {"
        "  background: shade(@theme_bg_color, 1.01);"
        "  border-bottom: 1px solid alpha(@theme_fg_color, 0.06);"
        "}"
        "#tbo-pages > header tabs tab {"
        "  margin: 4px 2px 0 2px;"
        "  padding: 7px 12px;"
        "  border-top-left-radius: 10px;"
        "  border-top-right-radius: 10px;"
        "}"
        "#tbo-sidebar {"
        "  background: shade(@theme_base_color, 0.985);"
        "  border-left: 1px solid alpha(@theme_fg_color, 0.08);"
        "}"
        "#tbo-status {"
        "  padding: 9px 12px;"
        "  border-top: 1px solid alpha(@theme_fg_color, 0.08);"
        "  background: shade(@theme_bg_color, 1.015);"
        "}"
        "#tbo-toolarea {"
        "  padding: 14px;"
        "}"
        ".tbo-sidebar-search {"
        "  min-height: 38px;"
        "  margin-bottom: 6px;"
        "}"
        ".tbo-sidebar-group, .tbo-sidebar-subgroup {"
        "  border-radius: 10px;"
        "  background: alpha(@theme_fg_color, 0.03);"
        "  border: 1px solid alpha(@theme_fg_color, 0.045);"
        "  padding: 4px 8px;"
        "}"
        ".tbo-sidebar-subgroup {"
        "  background: transparent;"
        "  border-color: transparent;"
        "  padding-left: 0;"
        "  padding-right: 0;"
        "}"
        ".tbo-asset-grid {"
        "  margin-top: 6px;"
        "  margin-bottom: 6px;"
        "}"
        ".tbo-asset-button {"
        "  border-radius: 12px;"
        "  padding: 7px;"
        "  background: alpha(@theme_fg_color, 0.01);"
        "}"
        ".tbo-asset-button:hover {"
        "  background: alpha(@theme_fg_color, 0.06);"
        "}"
        ".tbo-asset-button:focus-visible {"
        "  box-shadow: inset 0 0 0 2px @accent_bg_color;"
        "  background: alpha(@accent_bg_color, 0.08);"
        "}"
        ".tbo-dialog-content {"
        "  padding-top: 6px;"
        "}"
        ".tbo-dialog-card {"
        "  border-radius: 12px;"
        "  background: alpha(@theme_fg_color, 0.03);"
        "  border: 1px solid alpha(@theme_fg_color, 0.05);"
        "}";

    provider = gtk_css_provider_new ();
#if GTK_CHECK_VERSION(4, 12, 0)
    gtk_css_provider_load_from_string (provider, css);
#else
    gtk_css_provider_load_from_data (provider, css, -1);
#endif
    gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                                GTK_STYLE_PROVIDER (provider),
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (provider);
    loaded = TRUE;
}

static gboolean
tbo_window_apply_unmodified_key (TboWindow *tbo, guint keyval)
{
    TboDrawing *drawing = TBO_DRAWING (tbo->drawing);

    switch (keyval)
    {
        case GDK_KEY_plus:
            tbo_drawing_zoom_in (drawing);
            return TRUE;
        case GDK_KEY_minus:
            tbo_drawing_zoom_out (drawing);
            return TRUE;
        case GDK_KEY_1:
            tbo_drawing_zoom_100 (drawing);
            return TRUE;
        case GDK_KEY_2:
            tbo_drawing_zoom_fit (drawing);
            return TRUE;
        case GDK_KEY_s:
            tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
            return TRUE;
        case GDK_KEY_t:
            tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_TEXT);
            return TRUE;
        case GDK_KEY_d:
            tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_DOODLE);
            return TRUE;
        case GDK_KEY_b:
            tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_BUBBLE);
            return TRUE;
        case GDK_KEY_f:
            tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_FRAME);
            return TRUE;
        default:
            return FALSE;
    }
}

gboolean
tbo_window_handle_unmodified_key (TboWindow *tbo, guint keyval, GdkModifierType state)
{
    if (tbo == NULL || tbo->drawing == NULL)
        return FALSE;

    if (!tbo->key_binder || (state & (GDK_CONTROL_MASK | GDK_ALT_MASK | GDK_META_MASK)) != 0)
        return FALSE;

    return tbo_window_apply_unmodified_key (tbo, keyval);
}

static gboolean
on_key_cb (GtkEventControllerKey *controller,
           guint                  keyval,
           guint                  keycode,
           GdkModifierType        state,
           TboWindow             *tbo)
{
    TboToolBase *tool;
    TboKeyEvent event = { .keyval = keyval, .state = state };

    if (tbo->drawing == NULL || !gtk_widget_has_focus (GTK_WIDGET (tbo->drawing)))
        return FALSE;

    tool = tbo_toolbar_get_selected_tool (tbo->toolbar);
    if (tool)
        tool->on_key (tool, GTK_WIDGET (tbo->window), event);

    tbo_window_refresh_status (tbo);

    tbo_window_handle_unmodified_key (tbo, keyval, state);
    return FALSE;
}

static gboolean
global_key_cb (GtkEventControllerKey *controller,
               guint                  keyval,
               guint                  keycode,
               GdkModifierType        state,
               TboWindow             *tbo)
{
    GtkWidget *focus;

    if (keyval == GDK_KEY_Escape &&
        tbo->drawing != NULL &&
        tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) != NULL)
    {
        tbo_window_leave_frame (tbo);
        return TRUE;
    }

    focus = gtk_window_get_focus (GTK_WINDOW (tbo->window));
    if ((keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) &&
        tbo->drawing != NULL &&
        tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) == NULL)
    {
        TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
        Frame *selected_frame = tbo_tool_selector_get_selected_frame (selector);

        if (selected_frame != NULL &&
            (focus == NULL ||
             focus == tbo->drawing ||
             focus == tbo->dw_scroll ||
             focus == tbo->notebook ||
             gtk_widget_is_ancestor (focus, tbo->dw_scroll) ||
             gtk_widget_is_ancestor (focus, tbo->notebook)))
        {
            tbo_window_enter_frame (tbo, selected_frame);
            return TRUE;
        }
    }

    return FALSE;
}

TboWindow *
tbo_window_new (GtkWidget *window, GtkWidget *dw_scroll,
                GtkWidget *scroll2,
                GtkWidget *notebook, GtkWidget *toolarea,
                GtkWidget *status, GtkWidget *vbox, Comic *comic)
{
    TboWindow *tbo;

    tbo = malloc (sizeof (TboWindow));
    tbo->window = window;
    tbo->dw_scroll = dw_scroll;
    tbo->scroll2 = scroll2;
    tbo->drawing = tbo_scrolled_window_get_child (dw_scroll);
    tbo->status = status;
    tbo->vbox = vbox;
    tbo->menu_button = NULL;
    tbo->comic = comic;
    tbo->toolarea = toolarea;
    tbo->notebook = notebook;

    tbo->undo_stack = tbo_undo_stack_new ();
    tbo->path = NULL;
    tbo->browse_path = NULL;
    tbo->export_path = NULL;
    {
        gchar *recovery_dir = get_recovery_dir_path ();
        gchar *uuid = g_uuid_string_random ();

        tbo->autosave_path = g_build_filename (recovery_dir, uuid, NULL);
        {
            gchar *with_suffix = g_strconcat (tbo->autosave_path, ".tbo", NULL);
            g_free (tbo->autosave_path);
            tbo->autosave_path = with_suffix;
        }
        g_free (uuid);
        g_free (recovery_dir);
    }
    tbo->autosave_timeout_id = 0;
    tbo->syncing_page_reorder = FALSE;
    tbo->key_binder = TRUE;
    tbo->dirty = FALSE;
    tbo->destroying = FALSE;
    tbo->clean_state_id = 0;
    update_window_title (tbo);

    return tbo;
}

void 
tbo_window_free (TboWindow *tbo)
{
    if (tbo->autosave_timeout_id != 0)
        g_source_remove (tbo->autosave_timeout_id);
    detach_document_state (tbo);
    if (tbo->toolbar)
    {
        g_object_unref (tbo->toolbar);
        tbo->toolbar = NULL;
    }
    tbo_comic_free (tbo->comic);
    g_free (tbo->path);
    g_free (tbo->browse_path);
    g_free (tbo->export_path);
    g_free (tbo->autosave_path);
    tbo_undo_stack_del (tbo->undo_stack);
    free (tbo);
}

void
tbo_window_set_path (TboWindow *tbo, const gchar *path)
{
    set_window_path (&tbo->path, path);
    tbo_window_set_browse_path (tbo, path);
}

void
tbo_window_set_browse_path (TboWindow *tbo, const gchar *path)
{
    set_window_path (&tbo->browse_path, path);
    if (path != NULL)
        store_persisted_path ("browse_path", path);
}

void
tbo_window_set_export_path (TboWindow *tbo, const gchar *path)
{
    set_window_path (&tbo->export_path, path);
    if (path != NULL)
        store_persisted_path ("export_path", path);
}

gchar *
tbo_window_get_open_dir (TboWindow *tbo)
{
    if (tbo->browse_path == NULL)
        tbo->browse_path = load_persisted_path ("browse_path");

    if (tbo->browse_path != NULL)
        return get_dirname_or_home (tbo->browse_path);

    return get_dirname_or_home (tbo->path);
}

gchar *
tbo_window_get_export_dir (TboWindow *tbo)
{
    if (tbo->export_path == NULL)
        tbo->export_path = load_persisted_path ("export_path");

    if (tbo->export_path != NULL)
        return g_path_get_dirname (tbo->export_path);

    return tbo_window_get_open_dir (tbo);
}

void
tbo_window_mark_dirty (TboWindow *tbo)
{
    tbo->dirty = TRUE;
    update_window_title (tbo);
    schedule_autosave (tbo);
}

static void
update_dirty_state_from_history (TboWindow *tbo)
{
    if (tbo == NULL || tbo->undo_stack == NULL)
        return;

    if (tbo->undo_stack->current_state_id == tbo->clean_state_id)
        tbo_window_mark_clean (tbo);
    else
        tbo_window_mark_dirty (tbo);
}

void
tbo_window_mark_clean (TboWindow *tbo)
{
    tbo->dirty = FALSE;
    if (tbo->undo_stack != NULL)
        tbo->clean_state_id = tbo->undo_stack->current_state_id;
    update_window_title (tbo);
    if (tbo->autosave_timeout_id != 0)
    {
        g_source_remove (tbo->autosave_timeout_id);
        tbo->autosave_timeout_id = 0;
    }
    delete_recovery_files_for_window (tbo);
}

gboolean
tbo_window_has_unsaved_changes (TboWindow *tbo)
{
    return tbo->dirty;
}

gboolean
tbo_window_run_autosave (TboWindow *tbo)
{
    if (tbo == NULL || tbo->comic == NULL || !tbo->dirty || tbo->autosave_path == NULL)
        return FALSE;

    if (!tbo_comic_save_snapshot (tbo, tbo->autosave_path))
        return FALSE;

    write_recovery_metadata (tbo);
    return TRUE;
}

gboolean
tbo_window_recover_file (TboWindow *tbo, const gchar *autosave_file)
{
    gchar *source_path;

    if (tbo == NULL || autosave_file == NULL || *autosave_file == '\0')
        return FALSE;
    if (!g_file_test (autosave_file, G_FILE_TEST_EXISTS))
        return FALSE;

    source_path = load_recovery_source_path (autosave_file);
    if (!tbo_comic_open (tbo, (char *) autosave_file))
    {
        g_free (source_path);
        return FALSE;
    }

    set_window_path (&tbo->path, source_path);
    if (source_path != NULL)
        tbo_window_set_browse_path (tbo, source_path);
    else
        set_window_path (&tbo->browse_path, NULL);
    tbo_window_mark_dirty (tbo);
    tbo_window_delete_recovery_file (autosave_file);
    g_free (source_path);
    return TRUE;
}

gboolean
tbo_window_open_recent_project (TboWindow *tbo, const gchar *path)
{
    if (tbo == NULL || path == NULL || *path == '\0')
        return FALSE;
    if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
        remove_recent_project (path);
        tbo_alert_show (GTK_WINDOW (tbo->window), _("Couldn't open recent project"), _("The file no longer exists."));
        return FALSE;
    }
    if (!tbo_window_prepare_for_document_replace (tbo))
        return FALSE;

    if (!tbo_comic_open (tbo, (char *) path))
        return FALSE;

    tbo_window_add_recent_project (path);
    tbo_menu_refresh (tbo);
    return TRUE;
}

gboolean
tbo_window_reopen_last_project (TboWindow *tbo)
{
    gchar *last_project = tbo_window_get_last_project ();
    gboolean opened = FALSE;

    if (last_project != NULL)
        opened = tbo_window_open_recent_project (tbo, last_project);

    g_free (last_project);
    return opened;
}

TboThemeMode
tbo_window_get_theme_mode (void)
{
    return load_theme_mode_preference ();
}

void
tbo_window_set_theme_mode (TboWindow *tbo, TboThemeMode mode)
{
    GtkApplication *app;
    GList *windows;

    save_theme_mode_preference (mode);
    apply_theme_preferences ();

    if (tbo == NULL)
        return;

    app = gtk_window_get_application (GTK_WINDOW (tbo->window));
    if (app == NULL)
        return;

    for (windows = gtk_application_get_windows (app); windows != NULL; windows = windows->next)
    {
        GAction *action = g_action_map_lookup_action (G_ACTION_MAP (windows->data), "theme-mode");

        if (G_IS_SIMPLE_ACTION (action))
            g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_string (theme_mode_to_string (mode)));
    }
}

void
tbo_window_insert_page_widget (TboWindow *tbo, GtkWidget *page, Page *comic_page, gint nth)
{
    gint index = nth < 0 ? gtk_notebook_get_n_pages (GTK_NOTEBOOK (tbo->notebook)) : nth;

    g_object_set_data (G_OBJECT (page), TBO_PAGE_WIDGET_KEY, comic_page);
    gtk_notebook_insert_page (GTK_NOTEBOOK (tbo->notebook), page, create_page_tab_label (index), index);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (tbo->notebook), page, TRUE);
    refresh_page_tab_labels (tbo);
}

void
tbo_window_add_page_widget (TboWindow *tbo, GtkWidget *page, Page *comic_page)
{
    tbo_window_insert_page_widget (tbo, page, comic_page, -1);
}

gboolean
tbo_window_duplicate_current_page (TboWindow *tbo)
{
    Page *page;
    Page *cloned_page;
    gint index;

    if (tbo == NULL || tbo->comic == NULL)
        return FALSE;

    page = tbo_comic_get_current_page (tbo->comic);
    if (page == NULL)
        return FALSE;

    cloned_page = tbo_page_clone (page);
    if (cloned_page == NULL)
        return FALSE;

    index = tbo_comic_page_nth (tbo->comic, page) + 1;
    tbo_comic_insert_page (tbo->comic, cloned_page, index);
    tbo_window_insert_page_widget (tbo, create_darea (tbo), cloned_page, index);
    tbo_comic_set_current_page (tbo->comic, cloned_page);
    tbo_undo_stack_insert (tbo->undo_stack, tbo_action_page_add_new (tbo->comic, cloned_page, index));
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_window_mark_dirty (tbo);
    tbo_window_refresh_status (tbo);
    return TRUE;
}

void
tbo_window_remove_page_widget (TboWindow *tbo, gint nth)
{
    GtkWidget *page = get_page_widget (tbo, nth);

    if (page != NULL)
    {
        gtk_notebook_remove_page (GTK_NOTEBOOK (tbo->notebook), nth);
        refresh_page_tab_labels (tbo);
    }
}

gint
tbo_window_get_page_count (TboWindow *tbo)
{
    return gtk_notebook_get_n_pages (GTK_NOTEBOOK (tbo->notebook));
}

gboolean
tbo_window_close_request_cb (GtkWindow *window, TboWindow *tbo)
{
    TboConfirmCloseResult result = confirm_close (tbo);

    (void) window;

    if (result != TBO_CONFIRM_CLOSE_CANCEL)
    {
        if (result == TBO_CONFIRM_CLOSE_DISCARD)
            tbo_window_mark_clean (tbo);

        tbo_window_reset_document_state (tbo);
        tbo->destroying = TRUE;
        return FALSE;
    }

    return TRUE;
}


GtkWidget *
create_darea (TboWindow *tbo)
{
    GtkWidget *scrolled;
    GtkWidget *darea;

    scrolled = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    darea = tbo_drawing_new_with_params (tbo->comic);
    tbo_scrolled_window_set_child (scrolled, darea);
    setup_darea_controllers (darea, tbo);
    tbo_widget_show_all (scrolled);

    return scrolled;
}

TboWindow *
tbo_new_tbo (GtkApplication *app, int width, int height)
{
    const int sidebar_width = 300;
    const int window_width = MAX (width + sidebar_width + 80, 1100);
    const int window_height = MAX (height + 180, 720);
    TboWindow *tbo;
    Comic *comic;
    GtkWidget *window;
    GtkWidget *container;
    GtkWidget *tool_paned;
    GtkWidget *menu;
    GtkWidget *headerbar;
    TboToolbar *toolbar;
    GtkWidget *scrolled;
    GtkWidget *scrolled2;
    GtkWidget *darea;
    GtkWidget *status;
    GtkWidget *hpaned;
    GtkWidget *notebook;
    GtkEventController *global_key;

    window = app != NULL ? gtk_application_window_new (app) : gtk_window_new ();
    gtk_window_set_default_size (GTK_WINDOW (window), window_width, window_height);
    gchar *icon_path = tbo_get_data_path ("icon.png");
    g_free (icon_path);

    apply_theme_preferences ();
    load_app_css ();

    headerbar = gtk_header_bar_new ();
    gtk_header_bar_set_show_title_buttons (GTK_HEADER_BAR (headerbar), TRUE);
    gtk_window_set_titlebar (GTK_WINDOW (window), headerbar);
    container = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    tbo_widget_add_child (window, container);

    comic = tbo_comic_new (_("Untitled"), width, height);
    scrolled = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    darea = tbo_drawing_new_with_params (comic);
    tbo_scrolled_window_set_child (scrolled, darea);
    notebook = gtk_notebook_new ();
    gtk_widget_set_name (notebook, "tbo-pages");
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
    gtk_widget_set_hexpand (notebook, TRUE);
    gtk_widget_set_vexpand (notebook, TRUE);

    hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_hexpand (hpaned, TRUE);
    gtk_widget_set_vexpand (hpaned, TRUE);
    tool_paned = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name (tool_paned, "tbo-toolarea");
    scrolled2 = gtk_scrolled_window_new ();
    gtk_widget_set_name (scrolled2, "tbo-sidebar");
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    tbo_scrolled_window_set_child (scrolled2, tool_paned);
    gtk_widget_set_size_request (scrolled2, sidebar_width, -1);
    gtk_widget_set_vexpand (scrolled2, TRUE);

    gtk_paned_set_position (GTK_PANED (hpaned), window_width - sidebar_width);
    tbo_paned_pack_start (hpaned, notebook, TRUE, FALSE);
    tbo_paned_pack_end (hpaned, scrolled2, FALSE, FALSE);

    status = gtk_label_new (NULL);
    gtk_widget_set_name (status, "tbo-status");
    gtk_label_set_xalign (GTK_LABEL (status), 0.0);
    gtk_label_set_ellipsize (GTK_LABEL (status), PANGO_ELLIPSIZE_END);

    tbo = tbo_window_new (window, scrolled, scrolled2, notebook, tool_paned, status, container, comic);

    // Generando la barra de herramientas de la aplicacion
    toolbar = TBO_TOOLBAR (tbo_toolbar_new_with_params (tbo));
    tbo->toolbar = toolbar;

    setup_darea_controllers (darea, tbo);

    // key press event
    g_signal_connect (tbo->notebook, "switch-page", G_CALLBACK (notebook_switch_page_cb), tbo);
    g_signal_connect (tbo->notebook, "page-reordered", G_CALLBACK (notebook_page_reordered_cb), tbo);
    global_key = gtk_event_controller_key_new ();
    gtk_event_controller_set_propagation_phase (global_key, GTK_PHASE_CAPTURE);
    g_signal_connect (global_key, "key-pressed", G_CALLBACK (global_key_cb), tbo);
    gtk_widget_add_controller (window, global_key);
    g_signal_connect (window, "close-request", G_CALLBACK (tbo_window_close_request_cb), tbo);
    g_signal_connect (window, "destroy", G_CALLBACK (destroy_cb), tbo);

    menu = generate_menu (tbo);
    gtk_header_bar_pack_end (GTK_HEADER_BAR (headerbar), menu);
    tbo_box_pack_start (container, toolbar->toolbar, FALSE, FALSE, 0);

    tbo_widget_add_child (container, hpaned);

    tbo_box_pack_start (container, status, FALSE, FALSE, 0);

    tbo_widget_show_all (window);
    apply_window_icon (window);
    tbo_window_add_page_widget (tbo, scrolled, tbo_comic_get_current_page (comic));
    tbo_toolbar_set_selected_tool (toolbar, TBO_TOOLBAR_SELECTOR);

    tbo_window_refresh_status (tbo);
    return tbo;
}

TboWindow *
tbo_new_tbo_with_template (GtkApplication *app, int width, int height, TboComicTemplate template)
{
    TboWindow *tbo = tbo_new_tbo (app, width, height);

    tbo_window_apply_comic_template (tbo, template);
    return tbo;
}

void
tbo_window_refresh_status (TboWindow *tbo)
{
    if (tbo == NULL || tbo->destroying)
        return;

    update_statusbar (tbo);
    tbo_toolbar_update (tbo->toolbar);
}

void
tbo_empty_tool_area (TboWindow *tbo)
{
    tbo_widget_destroy_all_children (tbo->toolarea);
}

void
tbo_window_set_key_binder (TboWindow *tbo, gboolean keyb)
{
    tbo->key_binder = keyb;
    if (keyb)
        tbo_menu_enable_accel_keys (tbo);
    else
        tbo_menu_disable_accel_keys (tbo);
}

void
tbo_window_set_current_tab_page (TboWindow *tbo, gboolean setit)
{
    int nth;

    if (tbo == NULL || tbo->destroying)
        return;

    nth = tbo_comic_page_index (tbo->comic);
    if (setit)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (tbo->notebook), nth);
    tbo->dw_scroll = get_page_widget (tbo, nth);
    tbo->drawing = tbo_scrolled_window_get_child (tbo->dw_scroll);
    TBO_DRAWING (tbo->drawing)->tool = tbo->toolbar->selected_tool;

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (TBO_TOOL_SELECTOR (tbo->toolbar->selected_tool), NULL);
    tbo_tool_selector_set_selected_obj (TBO_TOOL_SELECTOR (tbo->toolbar->selected_tool), NULL);
}

void
tbo_window_enter_frame (TboWindow *tbo, Frame *frame)
{
    TboDrawing *drawing;
    TboToolSelector *selector;

    if (frame == NULL)
        return;

    drawing = TBO_DRAWING (tbo->drawing);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (selector, frame);
    tbo_tool_selector_set_selected_obj (selector, NULL);
    tbo_page_set_current_frame (tbo_comic_get_current_page (tbo->comic), frame);
    tbo_drawing_set_current_frame (drawing, frame);
    gtk_widget_grab_focus (tbo->drawing);
    tbo_tooltip_set (NULL, 0, 0, tbo);
    tbo_tooltip_set_center_timeout (_("press Esc to go back"), 3000, tbo);
    tbo_window_refresh_status (tbo);
    tbo_drawing_adjust_scroll (drawing);
}

void
tbo_window_reset_document_state (TboWindow *tbo)
{
    if (tbo == NULL)
        return;

    if (tbo->toolbar != NULL)
        tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_NONE);

    detach_document_state (tbo);

    if (tbo->drawing != NULL)
    {
        TBO_DRAWING (tbo->drawing)->tool = NULL;
        tbo_drawing_set_comic (TBO_DRAWING (tbo->drawing), NULL);
        tbo_drawing_set_current_frame (TBO_DRAWING (tbo->drawing), NULL);
    }

    tbo_window_set_key_binder (tbo, TRUE);
    tbo_tooltip_set (NULL, 0, 0, tbo);
}

void
tbo_window_leave_frame (TboWindow *tbo)
{
    TboDrawing *drawing;
    TboToolSelector *selector;
    Frame *frame;

    drawing = TBO_DRAWING (tbo->drawing);
    frame = tbo_drawing_get_current_frame (drawing);
    if (frame == NULL)
        return;

    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_drawing_set_current_frame (drawing, NULL);
    tbo_tool_selector_set_selected (selector, frame);
    tbo_tool_selector_set_selected_obj (selector, NULL);
    gtk_widget_grab_focus (tbo->drawing);
    tbo_tooltip_set (NULL, 0, 0, tbo);
    tbo_window_refresh_status (tbo);
    tbo_drawing_adjust_scroll (drawing);
}

gboolean
tbo_window_undo_cb (GtkWidget *widget, TboWindow *tbo) {
    gint old_page_count = tbo_window_get_page_count (tbo);

    (void) widget;

    tbo_undo_stack_undo (tbo->undo_stack);

    update_dirty_state_from_history (tbo);
    sync_page_widgets_with_comic (tbo);
    if (old_page_count != tbo_window_get_page_count (tbo) ||
        gtk_notebook_get_current_page (GTK_NOTEBOOK (tbo->notebook)) != tbo_comic_page_index (tbo->comic) ||
        get_page_widget (tbo, tbo_comic_page_index (tbo->comic)) != tbo->dw_scroll)
        tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    tbo_window_refresh_status (tbo);
    tbo_toolbar_update (tbo->toolbar);
    return FALSE;
}

gboolean
tbo_window_redo_cb (GtkWidget *widget, TboWindow *tbo) {
    gint old_page_count = tbo_window_get_page_count (tbo);

    (void) widget;

    tbo_undo_stack_redo (tbo->undo_stack);

    update_dirty_state_from_history (tbo);
    sync_page_widgets_with_comic (tbo);
    if (old_page_count != tbo_window_get_page_count (tbo) ||
        gtk_notebook_get_current_page (GTK_NOTEBOOK (tbo->notebook)) != tbo_comic_page_index (tbo->comic) ||
        get_page_widget (tbo, tbo_comic_page_index (tbo->comic)) != tbo->dw_scroll)
        tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    tbo_window_refresh_status (tbo);
    tbo_toolbar_update (tbo->toolbar);
    return FALSE;
}
