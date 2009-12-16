#include <stdio.h>
#include <gtk/gtk.h>

#include "ui-menu.h"
#include "ui-toolbar.h"
#include "ui-drawing.h"

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

int main (int argc, char**argv){
    gtk_init (&argc, &argv);

    GtkWidget *window;
    GtkWidget *container;
    GtkWidget *menu;
    GtkWidget *toolbar;
    GtkWidget *darea;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);
    gtk_window_set_icon (GTK_WINDOW (window), create_pixbuf ("icon.png"));

    g_signal_connect (window, "delete-event", G_CALLBACK (close_cb), NULL);

    // El contenedor principal
    container = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), container);

    // Generando el menu de la aplicacion
    menu = generate_menu ();
    gtk_box_pack_start (GTK_BOX (container), menu, FALSE, FALSE, 0);

    // Generando la barra de herramientas de la aplicacion
    toolbar = generate_toolbar ();
    gtk_box_pack_start (GTK_BOX (container), toolbar, FALSE, FALSE, 0);

    //darea = get_drawing_area();
    darea = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add (GTK_CONTAINER (darea), get_drawing_area(800, 450));
    gtk_container_add (GTK_CONTAINER (container), darea);

    gtk_widget_show_all (window);
    gtk_main ();

    return 0;
}

