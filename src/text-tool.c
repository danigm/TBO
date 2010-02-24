#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include "tbo-window.h"
#include "textobj.h"
#include "text-tool.h"
#include "frame.h"
#include "ui-drawing.h"

static GtkWidget *FONT = NULL;
static GtkWidget *FONT_COLOR = NULL;

GtkWidget *
setup_toolarea (TboWindow *tbo)
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *font_color_label = gtk_label_new (_("Font:"));
    GtkWidget *font_label = gtk_label_new (_("Text color:"));

    gtk_misc_set_alignment (GTK_MISC (font_label), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (font_color_label), 0, 0);

    FONT = gtk_font_button_new ();
    FONT_COLOR = gtk_color_button_new ();

    vbox = gtk_vbox_new (FALSE, 5);

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), font_label, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), FONT, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), font_color_label, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), FONT_COLOR, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);

    return vbox;
}

void text_tool_on_select (TboWindow *tbo)
{
    GtkWidget *toolarea = setup_toolarea (tbo);
    gtk_widget_show_all (GTK_WIDGET (toolarea));
    tbo_empty_tool_area (tbo);
    gtk_container_add (GTK_CONTAINER (tbo->toolarea), toolarea);
}

void text_tool_on_unselect (TboWindow *tbo)
{
    /*
    if (FONT)
        gtk_widget_destroy (GTK_WIDGET (FONT));
    if (FONT_COLOR)
        gtk_widget_destroy (GTK_WIDGET (FONT_COLOR));
    */
    tbo_empty_tool_area (tbo);
}

void text_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo)
{}

void text_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo)
{
    Frame *frame = get_frame_view ();
    double r, g, b;
    int x = (int)event->x;
    int y = (int)event->y;
    x = tbo_frame_get_base_x (x);
    y = tbo_frame_get_base_y (y);
    text_tool_get_color (&r, &g, &b);
    TextObj *text = tbo_text_new_width_params (x, y, 100, 0,
                                               "Texto",
                                               (char *)text_tool_get_font_name (),
                                               r, g, b);
    tbo_frame_add_obj (frame, text);
}

void text_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo)
{}

void text_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo)
{}

void text_tool_drawing (cairo_t *cr)
{
}

const gchar *
text_tool_get_font_name ()
{
    PangoFontDescription *pango_font = NULL;

    if (FONT)
    {
        pango_font = pango_font_description_from_string (
                gtk_font_button_get_font_name (GTK_FONT_BUTTON (FONT)));
        return pango_font_description_get_family (pango_font);
    }

    return NULL;
}

void
text_tool_get_color (double *r, double *g, double *b)
{
    GdkColor color;
    gtk_color_button_get_color (GTK_COLOR_BUTTON (FONT_COLOR), &color);
    *r = color.red / 65535.0;
    *g = color.green / 65535.0;
    *b = color.blue / 65535.0;
}
