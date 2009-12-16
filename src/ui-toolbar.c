#include <stdio.h>
#include <gtk/gtk.h>
#include "ui-toolbar.h"

gboolean toolbar_handler (GtkWidget *widget, GdkEvent *event, gpointer data){
    printf("toolbar\n");
}

void add_new_image_toolbar_item (GtkWidget *toolbar, const gchar *stock_id, void *cb, gpointer data){
    GtkToolItem *item;
    item = gtk_tool_button_new_from_stock (stock_id);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
    g_signal_connect (G_OBJECT (item), "clicked", G_CALLBACK (cb), data);
}

GtkWidget *generate_toolbar (){
    GtkWidget *toolbar;

    toolbar = gtk_toolbar_new ();
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
    
    add_new_image_toolbar_item (toolbar, GTK_STOCK_NEW, G_CALLBACK (toolbar_handler), NULL);
    add_new_image_toolbar_item (toolbar, GTK_STOCK_OPEN, G_CALLBACK (toolbar_handler), NULL);
    add_new_image_toolbar_item (toolbar, GTK_STOCK_SAVE, G_CALLBACK (toolbar_handler), NULL);

    return toolbar;
}

