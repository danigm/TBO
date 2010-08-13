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
        {DATA_DIR ICONDIR "selector.svg", TBO_STOCK_SELECTOR},
        {DATA_DIR ICONDIR "doodle.svg", TBO_STOCK_DOODLE},
        {DATA_DIR ICONDIR "text.svg", TBO_STOCK_TEXT},
        {DATA_DIR ICONDIR "pix.svg", TBO_STOCK_PIX},
        {DATA_DIR ICONDIR "bubble.svg", TBO_STOCK_BUBBLE},
    };

    int i;

    factory = gtk_icon_factory_new ();

    for (i=0; i<G_N_ELEMENTS (icons); i++)
    {
        image = (GdkPixbuf *) gdk_pixbuf_new_from_file (icons[i].image, &error);
        if (image == NULL)
        {
            printf (_("error loading image %s\n"), icons[i].image);
        }

        iconset = gtk_icon_set_new_from_pixbuf (image);
        gtk_icon_factory_add (factory, icons[i].stockid, iconset);
        g_object_unref (G_OBJECT(image));
        gtk_icon_set_unref (iconset);
    }

    gtk_icon_factory_add_default (factory);

    g_object_unref (G_OBJECT (factory));
}

