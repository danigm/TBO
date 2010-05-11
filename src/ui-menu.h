#ifndef __UI_MENU__
#define __UI_MENU__

#include <gtk/gtk.h>
#include "tbo-window.h"

GtkWidget *generate_menu (TboWindow *window);
void update_menubar (TboWindow *tbo);

#endif
