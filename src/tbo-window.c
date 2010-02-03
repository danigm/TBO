#include <stdio.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "tbo-window.h"
#include "comic.h"
#include "ui-menu.h"
#include "ui-toolbar.h"
#include "ui-drawing.h"
#include "tbo-types.h"

static int NWINDOWS = 0;

TboWindow *
tbo_window_new (GtkWidget *window, GtkWidget *dw_scroll, GtkWidget *toolarea,
                GtkWidget *status, GtkWidget *vbox, Comic *comic)
{
    TboWindow *tbo;
    GList *list;

    tbo = malloc (sizeof (TboWindow));
    tbo->window = window;
    tbo->dw_scroll = dw_scroll;
    list = gtk_container_get_children (GTK_CONTAINER (dw_scroll));
    tbo->drawing = GTK_WIDGET (list->data);
    tbo->status = status;
    tbo->vbox = vbox;
    tbo->comic = comic;
    tbo->toolarea = toolarea;

    return tbo;
}

void 
tbo_window_free (TboWindow *tbo)
{
    tbo_comic_free (tbo->comic);
    gtk_widget_destroy (tbo->window);
    free (tbo);
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

TboWindow *
tbo_new_tbo (int width, int height)
{
    TboWindow *tbo;
    Comic *comic;
    GtkWidget *window;
    GtkWidget *container;
    GtkWidget *tool_paned;
    GtkWidget *menu;
    GtkWidget *toolbar;
    GtkWidget *scrolled;
    GtkWidget *darea;
    GtkWidget *status;
    GtkWidget *hpaned;

    GtkUIManager *manager;

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
    darea = get_drawing_area (width, height);
    gtk_container_add (GTK_CONTAINER (scrolled), darea);

    hpaned = gtk_hpaned_new ();
    tool_paned = gtk_vbox_new (FALSE, 0);
    gtk_paned_set_position (GTK_PANED (hpaned), width - 200);
    gtk_paned_add1 (GTK_PANED (hpaned), scrolled);
    gtk_paned_add2 (GTK_PANED (hpaned), tool_paned);

    status = gtk_statusbar_new ();

    tbo = tbo_window_new (window, scrolled, tool_paned, status, container, comic);
    tbo_window_update_status (tbo, 0, 0);

    // ui-drawing.c (expose, motion and click)
    darea_connect_signals (tbo);

    g_signal_connect (window, "delete-event", G_CALLBACK (tbo_window_free_cb), tbo);

    // Generando la barra de herramientas de la aplicacion
    toolbar = generate_toolbar (tbo);

    // Generando el menu de la aplicacion
    menu = generate_menu (tbo);

    gtk_box_pack_start (GTK_BOX (container), menu, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (container), toolbar, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (container), hpaned);

    gtk_box_pack_start (GTK_BOX (container), status, FALSE, FALSE, 0);

    gtk_widget_show_all (window);

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
    update_drawing (tbo);
}

