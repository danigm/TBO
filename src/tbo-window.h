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


#ifndef __TBO_WINDOW__
#define __TBO_WINDOW__

#include <gtk/gtk.h>
#include "tbo-toolbar.h"
#include "tbo-types.h"
#include "tbo-undo.h"

typedef enum
{
    TBO_COMIC_TEMPLATE_EMPTY,
    TBO_COMIC_TEMPLATE_STRIP,
    TBO_COMIC_TEMPLATE_A4,
    TBO_COMIC_TEMPLATE_STORYBOARD,
    TBO_COMIC_TEMPLATE_N_TEMPLATES
} TboComicTemplate;

typedef enum
{
    TBO_THEME_MODE_SYSTEM,
    TBO_THEME_MODE_DARK,
    TBO_THEME_MODE_LIGHT,
} TboThemeMode;

struct _TboWindow
{
    GtkWidget *window;
    GtkWidget *dw_scroll;
    GtkWidget *scroll2;
    GtkWidget *toolarea;
    GtkWidget *notebook;
    GtkWidget *drawing;
    GtkWidget *status;
    GtkWidget *vbox;
    GtkWidget *menu_button;
    TboToolbar *toolbar;
    TboUndoStack *undo_stack;
    Comic *comic;
    gchar *path;
    gchar *browse_path;
    gchar *export_path;
    gchar *autosave_path;
    guint autosave_timeout_id;
    gboolean syncing_page_reorder;
    gboolean key_binder;
    gboolean dirty;
    gboolean destroying;
    guint64 clean_state_id;
};

TboWindow *tbo_window_new (GtkWidget *window, GtkWidget *dw_scroll, GtkWidget *scroll2, GtkWidget *notebook, GtkWidget *toolarea, GtkWidget *status, GtkWidget *vbox, Comic *comic);
void tbo_window_free (TboWindow *tbo);
gboolean tbo_window_close_request_cb (GtkWindow *window, TboWindow *tbo);
TboWindow * tbo_new_tbo (GtkApplication *app, int width, int height);
TboWindow * tbo_new_tbo_with_template (GtkApplication *app, int width, int height, TboComicTemplate template);
void tbo_comic_template_get_default_size (TboComicTemplate template, gint *width, gint *height);
void tbo_window_apply_comic_template (TboWindow *tbo, TboComicTemplate template);
void tbo_window_refresh_status (TboWindow *tbo);
void tbo_empty_tool_area (TboWindow *tbo);
void tbo_window_set_path (TboWindow *tbo, const gchar *path);
void tbo_window_set_browse_path (TboWindow *tbo, const gchar *path);
void tbo_window_set_export_path (TboWindow *tbo, const gchar *path);
gchar *tbo_window_get_open_dir (TboWindow *tbo);
gchar *tbo_window_get_export_dir (TboWindow *tbo);
gboolean tbo_window_prepare_for_document_replace (TboWindow *tbo);
void tbo_window_mark_dirty (TboWindow *tbo);
void tbo_window_mark_clean (TboWindow *tbo);
gboolean tbo_window_has_unsaved_changes (TboWindow *tbo);
gboolean tbo_window_run_autosave (TboWindow *tbo);
gboolean tbo_window_recover_file (TboWindow *tbo, const gchar *autosave_file);
gchar **tbo_window_list_recovery_files (gsize *n_files);
void tbo_window_delete_recovery_file (const gchar *autosave_file);
void tbo_window_clear_persisted_state (void);
void tbo_window_add_recent_project (const gchar *path);
gchar **tbo_window_get_recent_projects (gsize *n_projects);
gchar *tbo_window_get_last_project (void);
gboolean tbo_window_open_recent_project (TboWindow *tbo, const gchar *path);
gboolean tbo_window_reopen_last_project (TboWindow *tbo);
TboThemeMode tbo_window_get_theme_mode (void);
void tbo_window_set_theme_mode (TboWindow *tbo, TboThemeMode mode);
void tbo_window_add_page_widget (TboWindow *tbo, GtkWidget *page, Page *comic_page);
void tbo_window_insert_page_widget (TboWindow *tbo, GtkWidget *page, Page *comic_page, gint nth);
void tbo_window_remove_page_widget (TboWindow *tbo, gint nth);
gint tbo_window_get_page_count (TboWindow *tbo);
void tbo_window_set_current_tab_page (TboWindow *tbo, gboolean setit);
GtkWidget *create_darea (TboWindow *tbo);
gboolean tbo_window_duplicate_current_page (TboWindow *tbo);
void tbo_window_set_key_binder (TboWindow *tbo, gboolean keyb);
void tbo_window_enter_frame (TboWindow *tbo, Frame *frame);
void tbo_window_leave_frame (TboWindow *tbo);
void tbo_window_reset_document_state (TboWindow *tbo);
gboolean tbo_window_handle_unmodified_key (TboWindow *tbo, guint keyval, GdkModifierType state);
gboolean tbo_window_undo_cb (GtkWidget *widget, TboWindow *tbo);
gboolean tbo_window_redo_cb (GtkWidget *widget, TboWindow *tbo);


#endif
