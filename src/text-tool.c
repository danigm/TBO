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
static TextObj *TEXT_SELECTED = NULL;
static GtkTextBuffer *TEXT_BUFFER = NULL;

gboolean
on_tview_focus_in (GtkWidget *view, GdkEventFocus *event, gpointer data)
{
    set_key_binder (FALSE);
    return FALSE;
}

gboolean
on_tview_focus_out (GtkWidget *view, GdkEventFocus *event, gpointer data)
{
    set_key_binder (TRUE);
    return FALSE;
}


gboolean
on_text_change (GtkTextBuffer *buf, TboWindow *tbo)
{
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (buf, &start);
    gtk_text_buffer_get_end_iter (buf, &end);

    if (TEXT_SELECTED)
    {
        tbo_text_set_text (TEXT_SELECTED, gtk_text_buffer_get_text (buf, &start, &end, FALSE));
        update_drawing (tbo);
    }
    return FALSE;
}

gboolean
on_font_change (GtkFontButton *fbutton, TboWindow *tbo)
{
    if (TEXT_SELECTED)
    {
        tbo_text_change_font (TEXT_SELECTED, text_tool_get_pango_font ());
        update_drawing (tbo);
    }
    return FALSE;
}

gboolean
on_color_change (GtkColorButton *cbutton, TboWindow *tbo)
{
    if (TEXT_SELECTED)
    {
        double r, g, b;
        text_tool_get_color (&r, &g, &b);
        tbo_text_change_color (TEXT_SELECTED, r, g, b);
        update_drawing (tbo);
    }
    return FALSE;
}

GtkWidget *
setup_toolarea (TboWindow *tbo)
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *font_color_label = gtk_label_new (_("Text color:"));
    GtkWidget *font_label = gtk_label_new (_("Font:"));
    GtkWidget *scroll;
    GtkWidget *view;

    gtk_misc_set_alignment (GTK_MISC (font_label), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (font_color_label), 0, 0);

    FONT = gtk_font_button_new ();
    g_signal_connect (FONT, "font-set", G_CALLBACK (on_font_change), tbo);
    FONT_COLOR = gtk_color_button_new ();
    g_signal_connect (FONT_COLOR, "color-set", G_CALLBACK (on_color_change), tbo);

    vbox = gtk_vbox_new (FALSE, 5);

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), font_label, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), FONT, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), font_color_label, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (hbox), FONT_COLOR, TRUE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);

    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    view = gtk_text_view_new ();
    gtk_widget_add_events (view, GDK_FOCUS_CHANGE_MASK);
    g_signal_connect (view, "focus-in-event", G_CALLBACK (on_tview_focus_in), tbo);
    g_signal_connect (view, "focus-out-event", G_CALLBACK (on_tview_focus_out), tbo);

    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
    TEXT_BUFFER = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_set_text (TEXT_BUFFER, "", -1);
    g_signal_connect (TEXT_BUFFER, "changed", G_CALLBACK (on_text_change), tbo);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), view);
    gtk_box_pack_start (GTK_BOX (vbox), scroll, FALSE, FALSE, 5);

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
    set_key_binder (TRUE);
}

void text_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo)
{}

void text_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo)
{
    Frame *frame = get_frame_view ();
    double r, g, b;
    int x = (int)event->x;
    int y = (int)event->y;
    gboolean found = FALSE;
    GList *obj_list;
    tbo_object * obj;
    TextObj *text;

    for (obj_list = g_list_first (frame->objects); obj_list; obj_list = obj_list->next)
    {
        obj = (tbo_object *)obj_list->data;
        if (obj->type == TEXTOBJ && tbo_frame_point_inside_obj (obj, x, y))
        {
            text = (TextObj *)obj;
            found = TRUE;
        }
    }
    if (!found)
    {
        x = tbo_frame_get_base_x (x);
        y = tbo_frame_get_base_y (y);
        text_tool_get_color (&r, &g, &b);
        text = tbo_text_new_with_params (x, y, 100, 0,
                                          _("Texto"),
                                          (char *)text_tool_get_pango_font (),
                                          r, g, b);
        tbo_frame_add_obj (frame, text);
    }
    text_tool_set_selected (text);
    update_drawing (tbo);
}

void text_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo)
{}

void text_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo)
{}

void text_tool_drawing (cairo_t *cr)
{
    const double dashes[] = {5, 5};
    int x, y;

    if (TEXT_SELECTED)
    {
        tbo_object *OBJ = TEXT_SELECTED;
        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
        cairo_set_line_width (cr, 1);
        cairo_set_dash (cr, dashes, G_N_ELEMENTS (dashes), 0);
        cairo_set_source_rgb (cr, 0.9, 0, 0);
        int ox, oy, ow, oh;
        tbo_frame_get_obj_relative (OBJ, &ox, &oy, &ow, &oh);

        cairo_translate (cr, ox, oy);
        cairo_rotate (cr, OBJ->angle);
        cairo_rectangle (cr, 0, 0, ow, oh);
        cairo_stroke (cr);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
    }
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

char *
text_tool_get_pango_font ()
{
    if (FONT)
    {
        return (char *)gtk_font_button_get_font_name (GTK_FONT_BUTTON (FONT));
    }

    return "Sans Normal 27";
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

void
text_tool_set_selected (TextObj *text)
{
    GdkColor color;
    char *str = tbo_text_get_text (text);
    TEXT_SELECTED = NULL;
    gtk_font_button_set_font_name (GTK_FONT_BUTTON (FONT), tbo_text_get_string (text));
    tbo_text_get_color (text, &color);
    gtk_color_button_set_color (GTK_COLOR_BUTTON (FONT_COLOR), &color);
    gtk_text_buffer_set_text (TEXT_BUFFER, str, -1);
    TEXT_SELECTED = text;
}
