#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>
#include "ui-drawing.h"
#include "tbo-tooltip.h"
#include "tbo-window.h"

static GString *TOOLTIP = NULL;
static int X=0, Y=0;
static double ALPHA = 0.0;
static double INC_ALPHA = 0.10;
static gboolean DECREASING = FALSE;

void
cairo_rounded_rectangle (cairo_t *cr, int xx, int yy, int w, int h)
{
    double x             = xx,
           y             = yy,
           width         = w,
           height        = h,
           aspect        = 1.0,     /* aspect ratio */
           corner_radius = height / 10.0;   /* and corner curvature radius */

    double radius = corner_radius / aspect;
    double degrees = M_PI / 180.0;

    cairo_new_sub_path (cr);
    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);
}

gboolean
increase_alpha_cb (gpointer p)
{
    if (ALPHA >= 0.7)
    {
        ALPHA = 0.7;
        return FALSE;
    }

    ALPHA += INC_ALPHA;
    update_drawing ((TboWindow *)p);
    return TRUE;
}

gboolean
decrease_alpha_cb (gpointer p)
{
    if (!DECREASING)
        return FALSE;

    if (ALPHA <= 0.0 || !p)
    {
        ALPHA = 0.0;
        g_string_free (TOOLTIP, TRUE);
        TOOLTIP = NULL;
        update_drawing ((TboWindow *)p);
        return FALSE;
    }

    ALPHA -= INC_ALPHA;
    update_drawing ((TboWindow *)p);
    return TRUE;
}

void
tbo_tooltip_draw_background (cairo_t *cr, int w, int h)
{
    int margin = 5;

    cairo_set_source_rgba (cr, 0, 0, 0, ALPHA);
    cairo_rounded_rectangle (cr, -margin, -margin, w + margin * 2, h + margin * 2);
    cairo_fill (cr);
}

void
tbo_tooltip_set (const char *tooltip, int x, int y, TboWindow *tbo)
{

    if (!TOOLTIP)
    {
        if (tooltip)
        {
            TOOLTIP = g_string_new (tooltip);
            ALPHA = 0.0;
            g_timeout_add (50, increase_alpha_cb, tbo);
            DECREASING = FALSE;
            X = x;
            Y = y;
        }
    }
    else
    {
        // if it's the same passing
        if (x == X && y == Y && !strcmp (tooltip, TOOLTIP->str))
            return;

        if (!tooltip)
        {
            if (!DECREASING)
            {
                g_timeout_add (50, decrease_alpha_cb, tbo);
                DECREASING = TRUE;
            }
        }
        else
        {
            g_string_free (TOOLTIP, TRUE);

            TOOLTIP = g_string_new (tooltip);
            ALPHA = 0.0;
            g_timeout_add (50, increase_alpha_cb, tbo);
            DECREASING = FALSE;
            X = x;
            Y = y;
        }
    }
}

GString *
tbo_tooltip_get ()
{
    return TOOLTIP;
}

void
tbo_tooltip_draw (cairo_t *cr)
{
    if (!TOOLTIP)
        return;

    int w=0, h=0;
    int posx, posy;
    gchar *text = TOOLTIP->str;
    PangoLayout *layout;

    layout = pango_cairo_create_layout (cr);
    pango_layout_set_text (layout, text, -1);
    pango_layout_get_size (layout, &w, &h);
    w = (double)w / PANGO_SCALE;
    h = (double)h / PANGO_SCALE;

    //pango_layout_set_font_description (layout, desc);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);

    posx = X - w / 2;
    posy = Y - h / 2;
    cairo_translate (cr, posx, posy);

    tbo_tooltip_draw_background (cr, w, h);

    cairo_set_source_rgba (cr, 1, 1, 1, ALPHA);
    pango_cairo_show_layout (cr, layout);

    cairo_translate (cr, -posx, -posy);
}
