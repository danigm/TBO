#ifndef __TBO_WINDOW__
#define __TBO_WINDOW__

#include <gtk/gtk.h>
#include "tbo-types.h"

typedef struct
{
    GtkWidget *window;
    GtkWidget *dw_scroll;
    GtkWidget *toolarea;
    GtkWidget *notebook;
    GtkWidget *drawing;
    GtkWidget *status;
    GtkWidget *vbox;
    Comic *comic;
    char *path;
} TboWindow;

TboWindow *tbo_window_new (GtkWidget *window, GtkWidget *dw_scroll, GtkWidget *notebook, GtkWidget *toolarea, GtkWidget *status, GtkWidget *vbox, Comic *comic);
void tbo_window_free (TboWindow *tbo);
gboolean tbo_window_free_cb (GtkWidget *widget, GdkEventExpose *event, TboWindow *tbo);
GdkPixbuf *create_pixbuf (const gchar * filename);
TboWindow * tbo_new_tbo (int width, int height);
void tbo_window_update_status (TboWindow *tbo, int x, int y);
void tbo_empty_tool_area (TboWindow *tbo);
void tbo_window_set_path (TboWindow *tbo, const char *path);
GtkWidget *create_darea (TboWindow *tbo);

#endif
