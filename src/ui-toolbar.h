#ifndef __UI_TOOLBAR__
#define __UI_TOOLBAR__

#include <gtk/gtk.h>
#include "tbo-window.h"

enum Tool
{
    NONE,
    FRAME,
};

enum Tool get_selected_tool ();
void set_selected_tool (enum Tool tool);

GtkWidget *generate_toolbar (TboWindow *window);

#endif

