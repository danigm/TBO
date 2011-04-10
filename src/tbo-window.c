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
#include <malloc.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include "tbo-types.h"
#include "tbo-window.h"
#include "comic.h"
#include "page.h"
#include "ui-menu.h"
#include "tbo-toolbar.h"
#include "tbo-drawing.h"
#include "tbo-tool-selector.h"

static int NWINDOWS = 0;
static gboolean KEY_BINDER = TRUE;

static gboolean
notebook_switch_page_cb (GtkNotebook     *notebook,
                         gpointer        *page,
                         guint            page_num,
                         TboWindow        *tbo)
{
    tbo_comic_set_current_page_nth (tbo->comic, page_num);
    tbo_window_set_current_tab_page (tbo, FALSE);
    tbo_toolbar_update (tbo->toolbar);
    tbo_window_update_status (tbo, 0, 0);
    tbo_drawing_adjust_scroll (TBO_DRAWING (tbo->drawing));
    return FALSE;
}

static gboolean
on_key_cb (GtkWidget    *widget,
           GdkEventKey  *event,
           TboWindow    *tbo)
{
    TboToolBase *tool;
    TboDrawing *drawing = TBO_DRAWING (tbo->drawing);

    tool = tbo_toolbar_get_selected_tool (tbo->toolbar);
    if (tool)
        tool->on_key (tool, widget, event);

    tbo_window_update_status (tbo, 0, 0);

    if (KEY_BINDER)
    {
        switch (event->keyval)
        {
            case GDK_KEY_plus:
                tbo_drawing_zoom_in (drawing);
                break;
            case GDK_KEY_minus:
                tbo_drawing_zoom_out (drawing);
                break;
            case GDK_KEY_1:
                tbo_drawing_zoom_100 (drawing);
                break;
            case GDK_KEY_2:
                tbo_drawing_zoom_fit (drawing);
                break;
            case GDK_KEY_s:
                tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
                break;
            case GDK_KEY_t:
                tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_TEXT);
                break;
            case GDK_KEY_d:
                tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_DOODLE);
                break;
            case GDK_KEY_b:
                tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_BUBBLE);
                break;
            case GDK_KEY_f:
                tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_FRAME);
                break;
            default:
                break;
        }
    }
    return FALSE;
}

static gboolean
on_move_cb (GtkWidget     *widget,
           GdkEventMotion *event,
           TboWindow      *tbo)
{
    tbo_window_update_status (tbo, (int)event->x, (int)event->y);
    return FALSE;
}

TboWindow *
tbo_window_new (GtkWidget *window, GtkWidget *dw_scroll,
                GtkWidget *scroll2,
                GtkWidget *notebook, GtkWidget *toolarea,
                GtkWidget *status, GtkWidget *vbox, Comic *comic)
{
    TboWindow *tbo;
    GList *list;

    tbo = malloc (sizeof (TboWindow));
    tbo->window = window;
    tbo->dw_scroll = dw_scroll;
    tbo->scroll2 = scroll2;
    list = gtk_container_get_children (GTK_CONTAINER (dw_scroll));
    tbo->drawing = GTK_WIDGET (list->data);
    tbo->status = status;
    tbo->vbox = vbox;
    tbo->comic = comic;
    tbo->toolarea = toolarea;
    tbo->notebook = notebook;
    tbo->path = NULL;

    return tbo;
}

void 
tbo_window_free (TboWindow *tbo)
{
    tbo_comic_free (tbo->comic);
    gtk_widget_destroy (tbo->window);
    if (tbo->path)
        free (tbo->path);
    free (tbo);
}

void
tbo_window_set_path (TboWindow *tbo, const char *path)
{
    if (tbo->path)
        free (tbo->path);
    tbo->path = malloc (255 * sizeof (char));
    snprintf (tbo->path, 255, "%s", path);
}

gboolean 
tbo_window_free_cb (GtkWidget *widget, GdkEventExpose *event,
                    TboWindow *tbo)
{
    tbo_window_free (tbo);
    NWINDOWS--;
    if (!NWINDOWS)
        gtk_main_quit ();
    return FALSE;
}

GdkPixbuf *create_pixbuf (const gchar * filename)
{
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

GtkWidget *
create_darea (TboWindow *tbo)
{
    GtkWidget *scrolled;
    GtkWidget *darea;

    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    darea = tbo_drawing_new_with_params (tbo->comic);
    gtk_container_add (GTK_CONTAINER (scrolled), darea);
    tbo_drawing_init_dnd (TBO_DRAWING (darea), tbo);

    g_signal_connect_after (darea, "motion_notify_event", G_CALLBACK (on_move_cb), tbo);
    gtk_widget_show_all (scrolled);

    return scrolled;
}

TboWindow *
tbo_new_tbo (int width, int height)
{
    TboWindow *tbo;
    Comic *comic;
    GtkWidget *window;
    GtkWidget *container;
    GtkWidget *tool_paned;
    GtkWidget *menu;
    TboToolbar *toolbar;
    GtkWidget *scrolled;
    GtkWidget *scrolled2;
    GtkWidget *darea;
    GtkWidget *status;
    GtkWidget *hpaned;
    GtkWidget *notebook;

    NWINDOWS++;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (window), width, height);
    gtk_window_set_icon (GTK_WINDOW (window), create_pixbuf (DATA_DIR "/icon.png"));

    // El contenedor principal
    container = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), container);

    comic = tbo_comic_new (_("Untitled"), width, height);
    gtk_window_set_title (GTK_WINDOW (window), comic->title);
    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    darea = tbo_drawing_new_with_params (comic);
    gtk_container_add (GTK_CONTAINER (scrolled), darea);
    notebook = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolled, NULL);

    hpaned = gtk_hpaned_new ();
    tool_paned = gtk_vbox_new (FALSE, 0);
    scrolled2 = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled2), tool_paned);

    gtk_paned_set_position (GTK_PANED (hpaned), width - 200);
    gtk_paned_pack1 (GTK_PANED (hpaned), notebook, TRUE, FALSE);
    gtk_paned_pack2 (GTK_PANED (hpaned), scrolled2, TRUE, FALSE);

    status = gtk_statusbar_new ();

    tbo = tbo_window_new (window, scrolled, scrolled2, notebook, tool_paned, status, container, comic);

    // Generando la barra de herramientas de la aplicacion
    toolbar = TBO_TOOLBAR (tbo_toolbar_new_with_params (tbo));
    tbo->toolbar = toolbar;

    // drag & drop
    tbo_drawing_init_dnd (TBO_DRAWING (darea), tbo);

    // key press event
    g_signal_connect (tbo->notebook, "switch-page", G_CALLBACK (notebook_switch_page_cb), tbo);
    g_signal_connect (tbo->window, "key_press_event", G_CALLBACK (on_key_cb), tbo);
    g_signal_connect (window, "delete-event", G_CALLBACK (tbo_window_free_cb), tbo);

    // Generando el menu de la aplicacion
    menu = generate_menu (tbo);

    gtk_box_pack_start (GTK_BOX (container), menu, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (container), toolbar->toolbar, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (container), hpaned);

    gtk_box_pack_start (GTK_BOX (container), status, FALSE, FALSE, 0);

    gtk_widget_show_all (window);
    tbo_toolbar_set_selected_tool (toolbar, TBO_TOOLBAR_SELECTOR);

    tbo_window_update_status (tbo, 0, 0);
    return tbo;
}

void
tbo_window_update_status (TboWindow *tbo, int x, int y)
{
    char buffer[200];
    snprintf (buffer, 200, _("page: %d of %d [ %5d,%5d ] | frames: %d"),
                    tbo_comic_page_index (tbo->comic),
                    tbo_comic_len (tbo->comic),
                    x, y,
                    tbo_page_len (tbo_comic_get_current_page (tbo->comic)));
    gtk_statusbar_push (GTK_STATUSBAR (tbo->status), 0, buffer);
    tbo_toolbar_update (tbo->toolbar);
}

gboolean
remove_cb (GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy (widget);
    return FALSE;
}

void
tbo_empty_tool_area (TboWindow *tbo)
{
    gtk_container_foreach (GTK_CONTAINER (tbo->toolarea), (GtkCallback)remove_cb, NULL);
}

void
tbo_window_set_key_binder (TboWindow *tbo, gboolean keyb)
{
    KEY_BINDER = keyb;
    if (keyb)
        tbo_menu_enable_accel_keys (tbo);
    else
        tbo_menu_disable_accel_keys (tbo);
}

void
tbo_window_set_current_tab_page (TboWindow *tbo, gboolean setit)
{
    int nth;

    nth = tbo_comic_page_index (tbo->comic);
    if (setit)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (tbo->notebook), nth);
    tbo->dw_scroll = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tbo->notebook), nth);
    tbo->drawing = gtk_bin_get_child (GTK_BIN (tbo->dw_scroll));

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (TBO_TOOL_SELECTOR (tbo->toolbar->selected_tool), NULL);
    tbo_tool_selector_set_selected_obj (TBO_TOOL_SELECTOR (tbo->toolbar->selected_tool), NULL);
}
