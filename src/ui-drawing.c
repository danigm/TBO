#include <stdio.h>
#include <cairo.h>
#include <gtk/gtk.h>

#include "ui-drawing.h"

gboolean
on_expose_cb(GtkWidget      *widget,
             GdkEventExpose *event,
             gpointer       data)
{
    cairo_t *cr;
    int width, height;
    char *text = "TBO rulz!";
    cairo_text_extents_t extents;
    int *size = (int *) data;

    width = size[0];
    height = size[1];

    cr = gdk_cairo_create(GTK_LAYOUT (widget)->bin_window);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, size[0], size[1]);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle (cr, 0, 0, size[0], size[1]);
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
on_move_cb (GtkWidget      *widget,
           GdkEventExpose *event,
           gpointer       data)
{
    printf ("move\n");
}

gboolean
on_click_cb (GtkWidget      *widget,
           GdkEventExpose *event,
           gpointer       data)
{
    printf ("click\n");
}

GtkWidget *
get_drawing_area (int width, int height)
{

    GtkWidget *drawing;
    static int size[2];
    
    size[0] = width;
    size[1] = height;

    drawing = gtk_layout_new(NULL, NULL);
    gtk_layout_set_size(GTK_LAYOUT (drawing), size[0], size[1]);

    g_signal_connect(drawing, "expose-event",
            G_CALLBACK (on_expose_cb),  (gpointer) size);

    gtk_widget_add_events (drawing, GDK_BUTTON_PRESS_MASK |
                                    GDK_POINTER_MOTION_MASK);

    g_signal_connect (drawing, "button_press_event",
            G_CALLBACK (on_click_cb), NULL);

    g_signal_connect (drawing, "motion_notify_event",
            G_CALLBACK (on_move_cb), (gpointer) size);

    return drawing;
}

