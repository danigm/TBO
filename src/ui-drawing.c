#include <stdio.h>
#include <cairo.h>
#include <gtk/gtk.h>

#include "tbo-window.h"
#include "ui-drawing.h"

gboolean
on_expose_cb(GtkWidget      *widget,
             GdkEventExpose *event,
             TboWindow       *tbo)
{
    cairo_t *cr;
    int width, height;
    char *text = "TBO rulz!";
    cairo_text_extents_t extents;

    width = tbo->comic->width;
    height = tbo->comic->height;

    cr = gdk_cairo_create(GTK_LAYOUT (widget)->bin_window);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, width+2, height+2);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle (cr, 1, 1, width, height);
    cairo_stroke (cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 90.0);

    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, (width-extents.width)/2, (height+extents.height)/2);
    
    cairo_show_text(cr, text);

    cairo_destroy(cr);

    return FALSE;
}

gboolean
on_move_cb (GtkWidget     *widget,
           GdkEventMotion *event,
           TboWindow      *tbo)
{
    tbo_window_update_status (tbo, (int)event->x, (int)event->y);

    return FALSE;
}

gboolean
on_click_cb (GtkWidget    *widget,
           GdkEventButton *event,
           TboWindow      *tbo)
{
    return FALSE;
}

GtkWidget *
get_drawing_area (int width, int height)
{

    GtkWidget *drawing;

    drawing = gtk_layout_new(NULL, NULL);
    gtk_layout_set_size(GTK_LAYOUT (drawing), width, height);

    return drawing;
}

void
darea_connect_signals (TboWindow *tbo)
{
    GtkWidget *drawing;
    drawing = tbo->drawing;

    gtk_widget_add_events (drawing, GDK_BUTTON_PRESS_MASK |
                                    GDK_POINTER_MOTION_MASK);

    g_signal_connect(drawing, "expose-event",
            G_CALLBACK (on_expose_cb), tbo);

    g_signal_connect (drawing, "button_press_event",
            G_CALLBACK (on_click_cb), tbo);

    g_signal_connect (drawing, "motion_notify_event",
            G_CALLBACK (on_move_cb), tbo);
}

