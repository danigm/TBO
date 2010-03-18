#ifndef __UI_TOOLBAR__
#define __UI_TOOLBAR__

#include <gtk/gtk.h>
#include "tbo-window.h"

enum ToolSignal
{
    TOOL_SELECT,
    TOOL_UNSELECT,
    TOOL_MOVE,
    TOOL_CLICK,
    TOOL_RELEASE,
    TOOL_KEY,
    TOOL_DRAWING,
};

enum Tool
{
    NONE,
    SELECTOR,
    FRAME,
    DOODLE,
    TEXT,
};

typedef struct
{
    enum Tool tool;
    void (*tool_on_select) (TboWindow *);
    void (*tool_on_unselect) (TboWindow *);
    void (*tool_on_move) (GtkWidget *, GdkEventMotion *, TboWindow *);
    void (*tool_on_click) (GtkWidget *, GdkEventButton *, TboWindow *);
    void (*tool_on_release) (GtkWidget *, GdkEventButton *, TboWindow *);
    void (*tool_on_key) (GtkWidget *, GdkEventKey *, TboWindow *);
    void (*tool_drawing) (cairo_t *cr);
} ToolStruct;

void tool_signal (enum Tool tool, enum ToolSignal signal, gpointer data);

enum Tool get_selected_tool ();
void set_selected_tool (enum Tool tool, TboWindow *tbo);

GtkWidget *generate_toolbar (TboWindow *window);

gboolean notebook_switch_page_cb (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, TboWindow *tbo);

#endif

