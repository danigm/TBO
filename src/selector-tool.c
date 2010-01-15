#include <gtk/gtk.h>
#include <cairo.h>
#include "selector-tool.h"
#include "tbo-window.h"
#include "page.h"
#include "frame.h"
#include "comic.h"

#define R_SIZE 10

typedef struct
{
    double r;
    double g;
    double b;
} Color;

static Frame *SELECTED = NULL;
static int START_X=0, START_Y=0;
static int START_M_X=0, START_M_Y=0;
gboolean CLICKED = FALSE;
gboolean OVER_RESIZER = FALSE;

void
selector_tool_on_move (GtkWidget *widget,
        GdkEventMotion *event,
        TboWindow *tbo)
{
    int x, y, rx, ry;

    x = (int)event->x;
    y = (int)event->y;

    if (SELECTED != NULL)
    {
        // moving frame
        if (CLICKED)
        {
            SELECTED->x = START_M_X - (START_X - x);
            SELECTED->y = START_M_Y - (START_Y - y);
        }

        rx = SELECTED->x + SELECTED->width;
        ry = SELECTED->y + SELECTED->height;
        // over resizer
        if (((rx-R_SIZE) < x) &&
            ((rx+R_SIZE) > x) &&
            ((ry-R_SIZE) < y) &&
            ((ry+R_SIZE) > y))
        {
            OVER_RESIZER = TRUE;
        }
        else
        {
            OVER_RESIZER = FALSE;
        }
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
    Color border = {0.9, 0.9, 0};
    Color white = {1, 1, 1};
    Color black = {0, 0, 0};
    Color *resizer_border;
    Color *resizer_fill;
    int x, y;

    if (SELECTED != NULL)
    {
        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, G_N_ELEMENTS (dashes), 0);
        cairo_set_source_rgb (cr, border.r, border.g, border.b);
        cairo_rectangle (cr, SELECTED->x, SELECTED->y,
                SELECTED->width, SELECTED->height);
        cairo_stroke (cr);

        // resizer
        if (OVER_RESIZER)
        {
            resizer_fill = &black;
            resizer_border = &white;
        }
        else
        {
            resizer_fill = &white;
            resizer_border = &black;
        }

        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, 0, 0);

        x = SELECTED->x + SELECTED->width;
        y = SELECTED->y + SELECTED->height;

        cairo_rectangle (cr, x, y, R_SIZE, R_SIZE);
        cairo_set_source_rgb(cr, resizer_fill->r, resizer_fill->g, resizer_fill->b);
        cairo_fill (cr);

        cairo_set_source_rgb(cr, resizer_border->r, resizer_border->g, resizer_border->b);
        cairo_rectangle (cr, x, y, R_SIZE, R_SIZE);
        cairo_stroke (cr);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);

    }
}

Frame *
selector_tool_get_selected_frame ()
{
    return SELECTED;
}

