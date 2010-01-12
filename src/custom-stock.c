#include <gtk/gtk.h>
#include "custom-stock.h"

#define ICONDIR "/icons/"

typedef struct
{
    char *image;
    char *stockid;
} icon;

void load_custom_stock ()
{
    GtkIconFactory *factory;
    GtkIconSet *iconset;
    GdkPixbuf *image;
    GError *error = NULL;

    icon icons[] = {
        {DATA_DIR ICONDIR "frame.svg", TBO_STOCK_FRAME},
        {DATA_DIR ICONDIR "selector.svg", TBO_STOCK_SELECTOR}
    };

    int i;

    factory = gtk_icon_factory_new ();

    for (i=0; i<G_N_ELEMENTS (icons); i++)
    {
        image = (GdkPixbuf *) gdk_pixbuf_new_from_file (icons[i].image, &error);
        if (image == NULL)
        {
            printf ("error loading image %s\n", icons[i].image);
        }

        iconset = gtk_icon_set_new_from_pixbuf (image);
        gtk_icon_factory_add (factory, icons[i].stockid, iconset);
        g_object_unref (G_OBJECT(image));
        gtk_icon_set_unref (iconset);
    }

    gtk_icon_factory_add_default (factory);

    g_object_unref (G_OBJECT (factory));
}

