#include <gtk/gtk.h>

#include "tbo-window.h"

int main (int argc, char**argv){
    gtk_init (&argc, &argv);
    tbo_new_tbo (800, 450);
    gtk_main ();

    return 0;
}

