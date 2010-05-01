#include <gtk/gtk.h>
#include <glib.h>
#include <cairo.h>
#include "tbo-tooltip.h"

static GString *TOOLTIP = NULL;
static int X=0, Y=0;

void
tbo_tooltip_set (const char *tooltip, int x, int y)
{
    if (!TOOLTIP)
    {
        if (tooltip)
        {
            TOOLTIP = g_string_new (tooltip);
            X = x;
            Y = y;
        }
    }
    else
    {
        if (!tooltip)
        {
            g_string_free (TOOLTIP, TRUE);
            TOOLTIP = NULL;
        }
        else
        {
            TOOLTIP = g_string_overwrite (TOOLTIP, 0, tooltip);
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

    gchar *text = TOOLTIP->str;

    PangoLayout *layout;

    cairo_set_source_rgb(cr, 0, 0, 0);

    //TODO draw a background
    layout = pango_cairo_create_layout (cr);
    pango_layout_set_text (layout, text, -1);
    //pango_layout_set_font_description (layout, desc);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);

    cairo_translate (cr, X, Y);
    pango_cairo_show_layout (cr, layout);
    cairo_translate (cr, -X, -Y);
}
