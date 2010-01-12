#include <gtk/gtk.h>
#include "custom-stock.h"

#define FRAME "/icons/frame.svg"

//NEXTDO 
// incluir mas iconos
// generalizar esto

void load_custom_stock ()
{
    GtkIconFactory *factory;
    GtkIconSet *iconset;
    GdkPixbuf *image;
    GError *error = NULL;

    factory = gtk_icon_factory_new ();

    image = (GdkPixbuf *) gdk_pixbuf_new_from_file (DATA_DIR FRAME, &error);
    if (image == NULL)
    {
        printf ("error loading image %s\n", DATA_DIR "/icons/frame.svg");
    }

    iconset = gtk_icon_set_new_from_pixbuf (image);
    gtk_icon_factory_add (factory, "tbo-newframe", iconset);
    g_object_unref (G_OBJECT(image));
    gtk_icon_set_unref (iconset);

    gtk_icon_factory_add_default (factory);

    g_object_unref (G_OBJECT (factory));
}

