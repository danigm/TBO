#include <gtk/gtk.h>

#include "custom-stock.h"
#include "tbo-window.h"

int main (int argc, char**argv){
    gtk_init (&argc, &argv);
    load_custom_stock ();
    tbo_new_tbo (800, 450);
    gtk_main ();

    return 0;
}

