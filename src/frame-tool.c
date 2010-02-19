#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "frame-tool.h"
#include "tbo-window.h"
#include "frame.h"

static int N_FRAME_X = -1;
static int N_FRAME_Y = -1;
Frame *TMP_FRAME = NULL;

int
min (int x, int y)
{
    if (x < y)
        return x;
    else
        return y;
}

void
frame_tool_on_select (TboWindow *tbo)
{}

void
frame_tool_on_unselect (TboWindow *tbo)
{}

void
frame_tool_on_move (GtkWidget *widget,
        GdkEventMotion *event,
        TboWindow *tbo)
{
    int x, y;

    if (N_FRAME_X >= 0)
    {
        if (TMP_FRAME == NULL)
        {
            TMP_FRAME = tbo_frame_new (
                    N_FRAME_X,
                    N_FRAME_Y,
                    (int)fabs ((int)event->x - N_FRAME_X),
                    (int)fabs ((int)event->y - N_FRAME_Y));
        }
        else
        {
            x = (int)event->x;
            y = (int)event->y;
            TMP_FRAME->width = (int)fabs (x - N_FRAME_X);
            TMP_FRAME->height = (int)fabs (y - N_FRAME_Y);
            TMP_FRAME->x = min (N_FRAME_X, x);
            TMP_FRAME->y = min (N_FRAME_Y, y);
        }
    }
}

void
frame_tool_on_click (GtkWidget *widget,
        GdkEventButton *event,
        TboWindow *tbo)
{
    N_FRAME_X = (int)event->x;
    N_FRAME_Y = (int)event->y;
}

void
frame_tool_on_release (GtkWidget *widget,
        GdkEventButton *event,
        TboWindow *tbo)
{
    int w, h;

    w = (int)fabs (event->x - N_FRAME_X);
    h = (int)fabs (event->y - N_FRAME_Y);

    if (w != 0 && h != 0)
    {
        tbo_page_new_frame (tbo_comic_get_current_page (tbo->comic),
                min (N_FRAME_X, event->x), min (N_FRAME_Y, event->y),
                w, h);
    }

    N_FRAME_X = -1;
    N_FRAME_Y = -1;
    if (TMP_FRAME) tbo_frame_free (TMP_FRAME);
    TMP_FRAME = NULL;
}

void
frame_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo)
{}

Frame *
get_tmp_frame (){
    return TMP_FRAME;
}

void
frame_tool_drawing (cairo_t *cr)
{
    if (TMP_FRAME != NULL)
    {
        tbo_frame_draw_complete (TMP_FRAME, cr, 
                1, 1, 1,
                1, 0, 0,
                2);
    }
}

