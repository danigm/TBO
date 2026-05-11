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

#include "config.h"
#include "ui-menu.h"
#include "comic-new-dialog.h"
#include "comic-saveas-dialog.h"
#include "comic-open-dialog.h"
#include "tbo-window.h"
#include "tbo-drawing.h"
#include "export.h"
#include "tbo-tool-selector.h"
#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-base.h"
#include "tbo-object-group.h"
#include "tbo-undo.h"
#include "tbo-utils.h"
#include "tbo-widget.h"

struct menu_accel
{
    const gchar *action_name;
    const gchar * const *accels;
};

typedef struct
{
    const gchar *title;
    const gchar *accelerator;
} ShortcutEntry;

static const gchar *ACCEL_NEW[] = {"<Primary>n", NULL};
static const gchar *ACCEL_OPEN[] = {"<Primary>o", NULL};
static const gchar *ACCEL_SAVE[] = {"<Primary>s", NULL};
static const gchar *ACCEL_UNDO[] = {"<Primary>z", NULL};
static const gchar *ACCEL_REDO[] = {"<Primary>y", NULL};
static const gchar *ACCEL_CLONE[] = {"<Primary>d", NULL};
static const gchar *ACCEL_DELETE[] = {"Delete", NULL};
static const gchar *ACCEL_FLIP_H[] = {"h", NULL};
static const gchar *ACCEL_FLIP_V[] = {"v", NULL};
static const gchar *ACCEL_ORDER_UP[] = {"Page_Up", NULL};
static const gchar *ACCEL_ORDER_DOWN[] = {"Page_Down", NULL};
static const gchar *ACCEL_QUIT[] = {"<Primary>q", NULL};
static const gchar *ACCEL_SHORTCUTS[] = {"F1", NULL};

static const struct menu_accel MENU_ACCELS[] = {
    {"win.new", ACCEL_NEW},
    {"win.open", ACCEL_OPEN},
    {"win.save", ACCEL_SAVE},
    {"win.undo", ACCEL_UNDO},
    {"win.redo", ACCEL_REDO},
    {"win.clone", ACCEL_CLONE},
    {"win.delete", ACCEL_DELETE},
    {"win.flip-h", ACCEL_FLIP_H},
    {"win.flip-v", ACCEL_FLIP_V},
    {"win.order-up", ACCEL_ORDER_UP},
    {"win.order-down", ACCEL_ORDER_DOWN},
    {"win.quit", ACCEL_QUIT},
    {"win.shortcuts", ACCEL_SHORTCUTS},
};

static const ShortcutEntry FILE_SHORTCUTS[] = {
    {N_("New Comic"), "Ctrl+N"},
    {N_("Open Comic"), "Ctrl+O"},
    {N_("Save Comic"), "Ctrl+S"},
    {N_("Undo"), "Ctrl+Z"},
    {N_("Redo"), "Ctrl+Y"},
    {N_("Quit"), "Ctrl+Q"},
    {NULL, NULL},
};

static const ShortcutEntry TOOL_SHORTCUTS[] = {
    {N_("Selector"), "S"},
    {N_("New Frame"), "F"},
    {N_("Doodle"), "D"},
    {N_("Bubble"), "B"},
    {N_("Text"), "T"},
    {N_("Clone Selection"), "Ctrl+D"},
    {N_("Delete Selection"), "Delete"},
    {NULL, NULL},
};

static const ShortcutEntry VIEW_SHORTCUTS[] = {
    {N_("Zoom In"), "+"},
    {N_("Zoom Out"), "-"},
    {N_("Zoom 1:1"), "1"},
    {N_("Zoom Fit"), "2"},
    {N_("Enter Selected Frame"), "Enter"},
    {N_("Back to Page"), "Esc"},
    {NULL, NULL},
};

static const ShortcutEntry ARRANGE_SHORTCUTS[] = {
    {N_("Horizontal Flip"), "H"},
    {N_("Vertical Flip"), "V"},
    {N_("Move to Front"), "Page Up"},
    {N_("Move to Back"), "Page Down"},
    {NULL, NULL},
};

static GtkApplication *
get_app (TboWindow *tbo)
{
    return gtk_window_get_application (GTK_WINDOW (tbo->window));
}

static void
shortcuts_window_destroy_cb (GtkWidget *widget, gpointer user_data)
{
    TboWindow *tbo = user_data;

    g_object_set_data (G_OBJECT (tbo->window), "tbo-shortcuts-window", NULL);
    (void) widget;
}

static gboolean
shortcuts_key_pressed_cb (GtkEventControllerKey *controller,
                          guint                  keyval,
                          guint                  keycode,
                          GdkModifierType        state,
                          gpointer               user_data)
{
    (void) controller;
    (void) keycode;
    (void) state;

    if (keyval == GDK_KEY_Escape)
    {
        gtk_window_close (GTK_WINDOW (user_data));
        return TRUE;
    }

    return FALSE;
}

static GtkWidget *
create_shortcuts_section (const gchar *title, const ShortcutEntry *entries)
{
    GtkWidget *frame = gtk_frame_new (title);
    GtkWidget *grid = gtk_grid_new ();
    gint row;

    gtk_widget_add_css_class (frame, "tbo-dialog-card");
    gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 24);
    gtk_widget_set_margin_start (grid, 12);
    gtk_widget_set_margin_end (grid, 12);
    gtk_widget_set_margin_top (grid, 12);
    gtk_widget_set_margin_bottom (grid, 12);
    gtk_frame_set_child (GTK_FRAME (frame), grid);

    for (row = 0; entries[row].title != NULL; row++)
    {
        GtkWidget *label = gtk_label_new (_(entries[row].title));
        GtkWidget *accel = gtk_label_new (NULL);
        gchar *markup = g_markup_printf_escaped ("<tt>%s</tt>", entries[row].accelerator);

        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_label_set_xalign (GTK_LABEL (accel), 1.0);
        gtk_label_set_markup (GTK_LABEL (accel), markup);
        gtk_grid_attach (GTK_GRID (grid), label, 0, row, 1, 1);
        gtk_grid_attach (GTK_GRID (grid), accel, 1, row, 1, 1);
        g_free (markup);
    }

    return frame;
}

static void
show_shortcuts (TboWindow *tbo)
{
    GtkWidget *window;
    GtkWidget *headerbar;
    GtkWidget *scrolled;
    GtkWidget *content;
    GtkEventController *key_controller;

    window = g_object_get_data (G_OBJECT (tbo->window), "tbo-shortcuts-window");
    if (window != NULL)
    {
        gtk_window_present (GTK_WINDOW (window));
        return;
    }

    window = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (window), _("Keyboard Shortcuts"));
    gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (tbo->window));
    gtk_window_set_modal (GTK_WINDOW (window), TRUE);
    gtk_window_set_default_size (GTK_WINDOW (window), 520, 420);

    headerbar = gtk_header_bar_new ();
    gtk_header_bar_set_show_title_buttons (GTK_HEADER_BAR (headerbar), TRUE);
    gtk_window_set_titlebar (GTK_WINDOW (window), headerbar);

    key_controller = gtk_event_controller_key_new ();
    g_signal_connect (key_controller, "key-pressed", G_CALLBACK (shortcuts_key_pressed_cb), window);
    gtk_widget_add_controller (window, key_controller);

    scrolled = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_window_set_child (GTK_WINDOW (window), scrolled);

    content = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class (content, "tbo-dialog-content");
    gtk_widget_set_margin_start (content, 12);
    gtk_widget_set_margin_end (content, 12);
    gtk_widget_set_margin_top (content, 12);
    gtk_widget_set_margin_bottom (content, 12);
    tbo_scrolled_window_set_child (scrolled, content);

    tbo_widget_add_child (content, create_shortcuts_section (_("File"), FILE_SHORTCUTS));
    tbo_widget_add_child (content, create_shortcuts_section (_("Tools"), TOOL_SHORTCUTS));
    tbo_widget_add_child (content, create_shortcuts_section (_("View and Navigation"), VIEW_SHORTCUTS));
    tbo_widget_add_child (content, create_shortcuts_section (_("Arrange"), ARRANGE_SHORTCUTS));

    g_object_set_data (G_OBJECT (tbo->window), "tbo-shortcuts-window", window);
    g_signal_connect (window, "destroy", G_CALLBACK (shortcuts_window_destroy_cb), tbo);
    tbo_widget_show_all (window);
}

static void
toggle_menu_cb (GtkButton *button, gpointer user_data)
{
    GtkPopover *popover = GTK_POPOVER (g_object_get_data (G_OBJECT (button), "tbo-popover"));

    (void) user_data;

    if (popover == NULL)
        return;

    if (gtk_widget_get_visible (GTK_WIDGET (popover)))
        gtk_popover_popdown (popover);
    else
        gtk_popover_popup (popover);
}

static void
menu_button_destroy_cb (GtkWidget *button, gpointer user_data)
{
    GtkWidget *popover = g_object_get_data (G_OBJECT (button), "tbo-popover");

    (void) user_data;

    if (popover != NULL && gtk_widget_get_parent (popover) == button)
        gtk_widget_unparent (popover);
}

static GSimpleAction *
lookup_action (TboWindow *tbo, const gchar *name)
{
    return G_SIMPLE_ACTION (g_action_map_lookup_action (G_ACTION_MAP (tbo->window), name));
}

static void
set_action_enabled (TboWindow *tbo, const gchar *name, gboolean enabled)
{
    GSimpleAction *action = lookup_action (tbo, name);

    if (action != NULL)
        g_simple_action_set_enabled (action, enabled);
}

static void
set_accels_enabled (TboWindow *tbo, gboolean enabled)
{
    GtkApplication *app = get_app (tbo);
    static const gchar *EMPTY[] = {NULL};
    guint i;

    if (app == NULL)
        return;

    for (i = 0; i < G_N_ELEMENTS (MENU_ACCELS); i++)
    {
        gtk_application_set_accels_for_action (app,
                                               MENU_ACCELS[i].action_name,
                                               enabled ? MENU_ACCELS[i].accels : EMPTY);
    }
}

static void
clone_group_selection (TboWindow *tbo, TboToolSelector *selector, Frame *frame, TboObjectGroup *group, gboolean *cloned)
{
    GList *objects;
    TboObjectBase *last_cloned = NULL;

    for (objects = group->objs; objects != NULL; objects = objects->next)
    {
        TboObjectBase *object = TBO_OBJECT_BASE (objects->data);
        TboObjectBase *cloned_obj;

        if (object == NULL || object->clone == NULL)
            continue;

        cloned_obj = object->clone (object);
        if (cloned_obj == NULL)
            continue;

        cloned_obj->x += 10;
        cloned_obj->y -= 10;
        tbo_frame_add_obj (frame, cloned_obj);
        tbo_undo_stack_insert (tbo->undo_stack, tbo_action_object_add_new (frame, cloned_obj));
        last_cloned = cloned_obj;
        *cloned = TRUE;
    }

    if (last_cloned != NULL)
    {
        tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
        tbo_tool_selector_set_selected_obj (selector, last_cloned);
    }
}

static void
clone_selection (TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *frame = selector->selected_frame;
    Page *page = tbo_comic_get_current_page (tbo->comic);
    TboDrawing *drawing = TBO_DRAWING (tbo->drawing);

    gboolean cloned = FALSE;

    if (!tbo_drawing_get_current_frame (drawing) && frame)
    {
        Frame *cloned_frame = tbo_frame_clone (frame);
        tbo_frame_set_position (cloned_frame,
                                tbo_frame_get_x (cloned_frame) + 10,
                                tbo_frame_get_y (cloned_frame) - 10);
        tbo_page_add_frame (page, cloned_frame);
        tbo_undo_stack_insert (tbo->undo_stack, tbo_action_frame_add_new (page, cloned_frame));
        tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
        tbo_tool_selector_set_selected (selector, cloned_frame);
        cloned = TRUE;
    }
    else if (obj && tbo_drawing_get_current_frame (drawing))
    {
        if (TBO_IS_OBJECT_GROUP (obj))
        {
            clone_group_selection (tbo, selector, frame, TBO_OBJECT_GROUP (obj), &cloned);
        }
        else if (obj->clone != NULL)
        {
            TboObjectBase *cloned_obj = obj->clone (obj);

            if (cloned_obj != NULL)
            {
                cloned_obj->x += 10;
                cloned_obj->y -= 10;
                tbo_frame_add_obj (frame, cloned_obj);
                tbo_undo_stack_insert (tbo->undo_stack, tbo_action_object_add_new (frame, cloned_obj));
                tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
                tbo_tool_selector_set_selected_obj (selector, cloned_obj);
                cloned = TRUE;
            }
        }
    }

    if (cloned)
    {
        tbo_window_mark_dirty (tbo);
        if (!tbo_drawing_get_current_frame (drawing))
            tbo_window_refresh_status (tbo);
    }
    tbo_drawing_update (drawing);
}

static void
delete_selection (TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);

    if (tbo_tool_selector_delete_selected (selector))
    {
        tbo_window_mark_dirty (tbo);
        tbo_drawing_update (TBO_DRAWING (tbo->drawing));
    }
}

static void
flip_selection_h (TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *frame = selector->selected_frame;

    if (obj != NULL && frame != NULL)
    {
        tbo_object_base_fliph (obj);
        tbo_undo_stack_insert (tbo->undo_stack,
                               tbo_action_object_flags_new (obj,
                                                            obj->flipv,
                                                            !obj->fliph,
                                                            obj->flipv,
                                                            obj->fliph));
    }

    if (obj != NULL)
    {
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
    }
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
}

static void
flip_selection_v (TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *frame = selector->selected_frame;

    if (obj != NULL && frame != NULL)
    {
        tbo_object_base_flipv (obj);
        tbo_undo_stack_insert (tbo->undo_stack,
                               tbo_action_object_flags_new (obj,
                                                            !obj->flipv,
                                                            obj->fliph,
                                                            obj->flipv,
                                                            obj->fliph));
    }

    if (obj != NULL)
    {
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
    }
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
}

static void
order_selection_up (TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *frame = selector->selected_frame;
    gint index1;
    gint index2;

    if (obj != NULL && frame != NULL)
    {
        index1 = tbo_frame_object_nth (frame, obj);
        tbo_object_base_order_up (obj, frame);
        index2 = tbo_frame_object_nth (frame, obj);
        if (index1 != index2)
            tbo_undo_stack_insert (tbo->undo_stack, tbo_action_object_order_new (frame, obj, index1, index2));
    }

    if (obj != NULL)
    {
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
    }
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
}

static void
order_selection_down (TboWindow *tbo)
{
    TboToolSelector *selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    TboObjectBase *obj = selector->selected_object;
    Frame *frame = selector->selected_frame;
    gint index1;
    gint index2;

    if (obj != NULL && frame != NULL)
    {
        index1 = tbo_frame_object_nth (frame, obj);
        tbo_object_base_order_down (obj, frame);
        index2 = tbo_frame_object_nth (frame, obj);
        if (index1 != index2)
            tbo_undo_stack_insert (tbo->undo_stack, tbo_action_object_order_new (frame, obj, index1, index2));
    }

    if (obj != NULL)
    {
        tbo_window_mark_dirty (tbo);
        tbo_toolbar_update (tbo->toolbar);
    }
    tbo_drawing_update (TBO_DRAWING (tbo->drawing));
}

static void
open_tutorial (TboWindow *tbo)
{
    gchar *filename = tbo_get_data_path ("tut.tbo");

    if (tbo_window_prepare_for_document_replace (tbo) && tbo_comic_open (tbo, filename))
    {
        g_free (tbo->path);
        tbo->path = NULL;
    }

    g_free (filename);
}

static void
show_about (TboWindow *tbo)
{
    const gchar *authors[] = {
        "danigm <dani@danigm.net>",
	"",
        "Actualizado por Jaime, 2026",
        NULL
    };
    const gchar *artists[] = {"danigm <dani@danigm.net>",
			      "",
                              "Arcadia http://www.arcadiaproject.org :",
                              "Samuel Navas Portillo",
                              "Daniel Pavón Pérez",
                              "Juan Jesús Pérez Luna",
                              "",
                              "Zapatero y Rajoy:",
                              "Alfonso de Cala",
                              "",
                              "South park style:",
                              "Rafael García <bladecoder@gmail.com>",
                              "",
                              "Facilware:",
                              "VIcente Pons <simpons@gmail.com>",
                              NULL};

    gtk_show_about_dialog (GTK_WINDOW (tbo->window),
                           "name", _("TBO Comic Editor"),
                           "version", VERSION,
                           "logo-icon-name", "tbo",
                           "authors", authors,
                           "artists", artists,
                           "website", "https://github.com/danigm/TBO/",
                           "translator-credits", _("translator-credits"),
                           NULL);
}

static void action_new (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_comic_new_dialog (NULL, user_data); }
static void action_open (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_comic_open_dialog (NULL, user_data); }
static void action_open_recent (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_window_open_recent_project (user_data, g_variant_get_string (parameter, NULL)); }
static void action_reopen_last (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_window_reopen_last_project (user_data); }
static void action_duplicate_page (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_window_duplicate_current_page (user_data); }
static void action_save (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_comic_save_dialog (NULL, user_data); }
static void action_save_as (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_comic_saveas_dialog (NULL, user_data); }
static void action_export (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_export (user_data); }
static void action_undo (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_window_undo_cb (NULL, user_data); }
static void action_redo (GSimpleAction *action, GVariant *parameter, gpointer user_data) { tbo_window_redo_cb (NULL, user_data); }
static void action_clone (GSimpleAction *action, GVariant *parameter, gpointer user_data) { clone_selection (user_data); }
static void action_delete (GSimpleAction *action, GVariant *parameter, gpointer user_data) { delete_selection (user_data); }
static void action_flip_h (GSimpleAction *action, GVariant *parameter, gpointer user_data) { flip_selection_h (user_data); }
static void action_flip_v (GSimpleAction *action, GVariant *parameter, gpointer user_data) { flip_selection_v (user_data); }
static void action_order_up (GSimpleAction *action, GVariant *parameter, gpointer user_data) { order_selection_up (user_data); }
static void action_order_down (GSimpleAction *action, GVariant *parameter, gpointer user_data) { order_selection_down (user_data); }
static void action_tutorial (GSimpleAction *action, GVariant *parameter, gpointer user_data) { open_tutorial (user_data); }
static void action_shortcuts (GSimpleAction *action, GVariant *parameter, gpointer user_data) { show_shortcuts (user_data); }
static void action_theme_mode_change_state (GSimpleAction *action, GVariant *value, gpointer user_data)
{
    TboThemeMode mode = TBO_THEME_MODE_SYSTEM;
    const gchar *mode_name = g_variant_get_string (value, NULL);

    if (g_strcmp0 (mode_name, "dark") == 0)
        mode = TBO_THEME_MODE_DARK;
    else if (g_strcmp0 (mode_name, "light") == 0)
        mode = TBO_THEME_MODE_LIGHT;

    tbo_window_set_theme_mode (user_data, mode);
    g_simple_action_set_state (action, value);
}
static void action_about (GSimpleAction *action, GVariant *parameter, gpointer user_data) { show_about (user_data); }
static void action_quit (GSimpleAction *action, GVariant *parameter, gpointer user_data) { gtk_window_close (GTK_WINDOW (((TboWindow *) user_data)->window)); }

static void
install_actions (TboWindow *tbo)
{
    static const GActionEntry entries[] = {
        {"new", action_new, NULL, NULL, NULL},
        {"open", action_open, NULL, NULL, NULL},
        {"open-recent", action_open_recent, "s", NULL, NULL},
        {"reopen-last", action_reopen_last, NULL, NULL, NULL},
        {"duplicate-page", action_duplicate_page, NULL, NULL, NULL},
        {"save", action_save, NULL, NULL, NULL},
        {"save-as", action_save_as, NULL, NULL, NULL},
        {"export", action_export, NULL, NULL, NULL},
        {"undo", action_undo, NULL, NULL, NULL},
        {"redo", action_redo, NULL, NULL, NULL},
        {"clone", action_clone, NULL, NULL, NULL},
        {"delete", action_delete, NULL, NULL, NULL},
        {"flip-h", action_flip_h, NULL, NULL, NULL},
        {"flip-v", action_flip_v, NULL, NULL, NULL},
        {"order-up", action_order_up, NULL, NULL, NULL},
        {"order-down", action_order_down, NULL, NULL, NULL},
        {"tutorial", action_tutorial, NULL, NULL, NULL},
        {"shortcuts", action_shortcuts, NULL, NULL, NULL},
        {"about", action_about, NULL, NULL, NULL},
        {"quit", action_quit, NULL, NULL, NULL},
    };

    g_action_map_add_action_entries (G_ACTION_MAP (tbo->window),
                                     entries,
                                     G_N_ELEMENTS (entries),
                                     tbo);

    {
        GSimpleAction *theme_mode = g_simple_action_new_stateful ("theme-mode",
                                                                  G_VARIANT_TYPE_STRING,
                                                                  g_variant_new_string (
                                                                      tbo_window_get_theme_mode () == TBO_THEME_MODE_DARK ? "dark" :
                                                                      tbo_window_get_theme_mode () == TBO_THEME_MODE_LIGHT ? "light" :
                                                                      "system"));
        g_signal_connect (theme_mode, "change-state", G_CALLBACK (action_theme_mode_change_state), tbo);
        g_action_map_add_action (G_ACTION_MAP (tbo->window), G_ACTION (theme_mode));
    }
}

static GMenuModel *
build_menu_model (TboWindow *window)
{
    GMenu *root = g_menu_new ();
    GMenu *section;
    gsize recent_count = 0;
    gchar **recent_paths;
    gchar *last_project;
    gsize i;

    section = g_menu_new ();
    g_menu_append (section, _("New"), "win.new");
    g_menu_append (section, _("Open"), "win.open");
    last_project = tbo_window_get_last_project ();
    if (last_project != NULL)
        g_menu_append (section, _("Reopen Last Project"), "win.reopen-last");
    g_menu_append (section, _("Duplicate Page"), "win.duplicate-page");
    g_menu_append (section, _("Save"), "win.save");
    g_menu_append (section, _("Save As"), "win.save-as");
    g_menu_append (section, _("Export"), "win.export");

    recent_paths = tbo_window_get_recent_projects (&recent_count);
    if (recent_count > 0)
    {
        GMenu *recent = g_menu_new ();

        for (i = 0; i < recent_count; i++)
        {
            GMenuItem *item;
            gchar *basename;

            basename = g_path_get_basename (recent_paths[i]);
            item = g_menu_item_new (basename, NULL);
            g_menu_item_set_action_and_target_value (item,
                                                     "win.open-recent",
                                                     g_variant_new_string (recent_paths[i]));
            g_menu_append_item (recent, item);
            g_object_unref (item);
            g_free (basename);
        }

        g_menu_append_submenu (section, _("Open Recent"), G_MENU_MODEL (recent));
        g_object_unref (recent);
    }

    g_menu_append_section (root, NULL, G_MENU_MODEL (section));
    g_object_unref (section);
    g_strfreev (recent_paths);
    g_free (last_project);

    section = g_menu_new ();
    g_menu_append (section, _("Undo"), "win.undo");
    g_menu_append (section, _("Redo"), "win.redo");
    g_menu_append (section, _("Clone"), "win.clone");
    g_menu_append (section, _("Delete"), "win.delete");
    g_menu_append (section, _("Horizontal Flip"), "win.flip-h");
    g_menu_append (section, _("Vertical Flip"), "win.flip-v");
    g_menu_append (section, _("Move to Front"), "win.order-up");
    g_menu_append (section, _("Move to Back"), "win.order-down");
    g_menu_append_section (root, NULL, G_MENU_MODEL (section));
    g_object_unref (section);

    section = g_menu_new ();
    {
        GMenu *theme_menu = g_menu_new ();
        GMenuItem *item;

        item = g_menu_item_new (_("Follow System Theme"), NULL);
        g_menu_item_set_action_and_target_value (item, "win.theme-mode", g_variant_new_string ("system"));
        g_menu_append_item (theme_menu, item);
        g_object_unref (item);

        item = g_menu_item_new (_("Dark Theme"), NULL);
        g_menu_item_set_action_and_target_value (item, "win.theme-mode", g_variant_new_string ("dark"));
        g_menu_append_item (theme_menu, item);
        g_object_unref (item);

        item = g_menu_item_new (_("Light Theme"), NULL);
        g_menu_item_set_action_and_target_value (item, "win.theme-mode", g_variant_new_string ("light"));
        g_menu_append_item (theme_menu, item);
        g_object_unref (item);

        g_menu_append_submenu (section, _("Theme"), G_MENU_MODEL (theme_menu));
        g_object_unref (theme_menu);
    }
    g_menu_append (section, _("Keyboard Shortcuts"), "win.shortcuts");
    g_menu_append (section, _("Tutorial"), "win.tutorial");
    g_menu_append (section, _("About"), "win.about");
    g_menu_append (section, _("Quit"), "win.quit");
    g_menu_append_section (root, NULL, G_MENU_MODEL (section));
    g_object_unref (section);

    return G_MENU_MODEL (root);
}

void
update_menubar (TboWindow *tbo)
{
    gboolean active = FALSE;
    gboolean object_selected = FALSE;
    TboDrawing *drawing;
    TboToolSelector *selector;
    TboObjectBase *obj;
    Frame *frame;

    if (tbo->toolbar == NULL || tbo->destroying)
        return;

    drawing = TBO_DRAWING (tbo->drawing);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    obj = selector->selected_object;
    frame = selector->selected_frame;

    if (tbo_drawing_get_current_frame (drawing) && obj)
    {
        active = TRUE;
        object_selected = TRUE;
    }
    else if (!tbo_drawing_get_current_frame (drawing) && frame)
    {
        active = TRUE;
    }

    set_action_enabled (tbo, "undo", tbo_undo_active_undo (tbo->undo_stack));
    set_action_enabled (tbo, "redo", tbo_undo_active_redo (tbo->undo_stack));
    set_action_enabled (tbo, "clone", active);
    set_action_enabled (tbo, "delete", active);
    set_action_enabled (tbo, "flip-h", object_selected);
    set_action_enabled (tbo, "flip-v", object_selected);
    set_action_enabled (tbo, "order-up", object_selected);
    set_action_enabled (tbo, "order-down", object_selected);
}

GtkWidget *
generate_menu (TboWindow *window)
{
    GtkWidget *button;
    GtkWidget *icon;

    install_actions (window);
    set_accels_enabled (window, TRUE);

    button = gtk_button_new ();
    icon = gtk_image_new_from_icon_name ("open-menu-symbolic");
    gtk_image_set_pixel_size (GTK_IMAGE (icon), 16);
    gtk_button_set_child (GTK_BUTTON (button), icon);
    gtk_widget_set_focusable (button, TRUE);
    gtk_widget_set_tooltip_text (button, _("Menu"));
    g_signal_connect (button, "clicked", G_CALLBACK (toggle_menu_cb), NULL);
    g_signal_connect (button, "destroy", G_CALLBACK (menu_button_destroy_cb), NULL);
    window->menu_button = button;
    tbo_menu_refresh (window);

    update_menubar (window);
    return button;
}

void
tbo_menu_refresh (TboWindow *tbo)
{
    GMenuModel *model;
    GtkWidget *popover;
    GtkWidget *old_popover;

    if (tbo == NULL || tbo->menu_button == NULL)
        return;

    old_popover = g_object_get_data (G_OBJECT (tbo->menu_button), "tbo-popover");
    if (old_popover != NULL && gtk_widget_get_parent (old_popover) == tbo->menu_button)
        gtk_widget_unparent (old_popover);

    model = build_menu_model (tbo);
    popover = gtk_popover_menu_new_from_model (model);
    gtk_widget_set_parent (popover, tbo->menu_button);
    gtk_popover_set_position (GTK_POPOVER (popover), GTK_POS_BOTTOM);
    g_object_set_data (G_OBJECT (tbo->menu_button), "tbo-popover", popover);
    g_object_unref (model);
}

void
tbo_menu_enable_accel_keys (TboWindow *tbo)
{
    set_accels_enabled (tbo, TRUE);
}

void
tbo_menu_disable_accel_keys (TboWindow *tbo)
{
    set_accels_enabled (tbo, FALSE);
}
