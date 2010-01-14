#include <gtk/gtk.h>
#include <cairo.h>
#include "selector-tool.h"
#include "tbo-window.h"
#include "page.h"
#include "frame.h"
#include "comic.h"

static Frame *SELECTED = NULL;
static int START_X=0, START_Y=0;
static int START_M_X=0, START_M_Y=0;
gboolean CLICKED = FALSE;

void
selector_tool_on_move (GtkWidget *widget,
        GdkEventMotion *event,
        TboWindow *tbo)
{
    int x, y;
    x = (int)event->x;
    y = (int)event->y;
    if (SELECTED != NULL && CLICKED)
    {
        SELECTED->x = START_M_X - (START_X - x);
        SELECTED->y = START_M_Y - (START_Y - y);
    }
}

void
selector_tool_on_click (GtkWidget *widget,
        GdkEventButton *event,
        TboWindow *tbo)
{
    int x, y;
    GList *frame_list;
    Page *page;
    Frame *frame;

    x = (int)event->x;
    y = (int)event->y;

    page = tbo_comic_get_current_page (tbo->comic);
    for (frame_list = page->frames; frame_list; frame_list = frame_list->next)
    {
        frame = (Frame *)frame_list->data;
        if (tbo_frame_point_inside (frame, x, y))
        {
            // Selecting last occurrence.
            SELECTED = frame;
        }
    }
    START_X = x;
    START_Y = y;
    if (SELECTED != NULL)
    {
        START_M_X = SELECTED->x;
        START_M_Y = SELECTED->y;
    }
    CLICKED = TRUE;
}

void
selector_tool_on_release (GtkWidget *widget,
        GdkEventButton *event,
        TboWindow *tbo)
{
    START_X = 0;
    START_Y = 0;
    CLICKED = FALSE;
}

void
selector_tool_drawing (cairo_t *cr)
{
    const double dashes[] = {5, 5};
    double border_r=0.9, border_g=0.9, border_b=0;

    if (SELECTED != NULL)
    {
        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, G_N_ELEMENTS (dashes), 0);
        cairo_set_source_rgba(cr, border_r, border_g, border_b, 1);
        cairo_rectangle(cr, SELECTED->x, SELECTED->y,
                SELECTED->width, SELECTED->height);
        cairo_stroke (cr);
        cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
    }
}

Frame *
selector_tool_get_selected_frame ()
{
    return SELECTED;
}

