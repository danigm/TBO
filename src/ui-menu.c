#include <stdio.h>
#include <gtk/gtk.h>

#include "ui-menu.h"

gboolean menu_handler (GtkWidget *widget, GdkEvent *event, gpointer data){
    printf ("Menu\n");
}

gboolean close_cb (GtkWidget *widget, GdkEvent *event, gpointer data){
    printf ("Ventana cerrada\n");
    gtk_main_quit ();
}

void add_new_menu_item (GtkWidget *menu, const char *label, void *cb, gpointer data){
    GtkWidget *item;
    item = gtk_menu_item_new_with_label (label);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_signal_connect_swapped (G_OBJECT (item), "activate", G_CALLBACK(cb), data);
}

void add_new_image_menu_item (GtkWidget *menu, const gchar *stock_id, void *cb, gpointer data){
    GtkWidget *item;
    item = gtk_image_menu_item_new_from_stock (stock_id, gtk_accel_group_new ());
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_signal_connect_swapped (G_OBJECT (item), "activate", G_CALLBACK(cb), data);
}

GtkWidget *generate_menu (){
    GtkWidget *menu;
    GtkWidget *file_item, *menu_file;
    
    menu = gtk_menu_bar_new ();

    menu_file = gtk_menu_new ();   
    add_new_image_menu_item(menu_file, GTK_STOCK_NEW, G_CALLBACK(menu_handler), NULL);
    add_new_image_menu_item(menu_file, GTK_STOCK_OPEN, G_CALLBACK(menu_handler), NULL);
    add_new_image_menu_item(menu_file, GTK_STOCK_SAVE, G_CALLBACK(menu_handler), NULL);
    add_new_image_menu_item(menu_file, GTK_STOCK_QUIT, G_CALLBACK(close_cb), NULL);

    file_item = gtk_menu_item_new_with_label ("Archivo");
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_item), menu_file);
    gtk_menu_bar_append (GTK_MENU_BAR (menu), file_item);

    return menu;
}

