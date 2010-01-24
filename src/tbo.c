#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "config.h"

#include "custom-stock.h"
#include "tbo-window.h"

int main (int argc, char**argv){

#ifdef ENABLE_NLS
	/* Initialize the i18n stuff */
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

    gtk_init (&argc, &argv);
    load_custom_stock ();
    tbo_new_tbo (800, 450);
    gtk_main ();

    return 0;
}

