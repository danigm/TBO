#ifndef __TBO_DND__
#define __TBO_DND__

#include <gtk/gtk.h>
#include "tbo-window.h"

enum {
    TARGET_STRING,
};


static GtkTargetEntry TARGET_LIST[] = {
    { "STRING",     0, TARGET_STRING },
    { "text/plain", 0, TARGET_STRING },
};

static guint N_TARGETS = G_N_ELEMENTS (TARGET_LIST);

// destination signals
void drag_data_received_handl (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint target_type, guint time, TboWindow *tbo);

// source signals
void drag_data_get_handl (GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint target_type, guint time, char *svg);
void drag_begin_handl (GtkWidget *widget, GdkDragContext *context, char *svg);
void drag_end_handl (GtkWidget *widget, GdkDragContext *context, gpointer user_data);

#endif
