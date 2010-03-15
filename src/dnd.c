#include <string.h>
#include <gtk/gtk.h>
#include "dnd.h"
#include "ui-drawing.h"
#include "frame.h"
#include "svgimage.h"
#include "tbo-window.h"

static GtkWidget *DND_IMAGE = NULL;

void
drag_data_received_handl (GtkWidget *widget,
                          GdkDragContext *context,
                          gint x, gint y,
                          GtkSelectionData *selection_data,
                          guint target_type,
                          guint time,
                          TboWindow *tbo)
{
    float zoom = tbo_drawing_get_zoom ();
    glong   *_idata;
    gchar   *_sdata;

    gboolean dnd_success = FALSE;
    gboolean delete_selection_data = FALSE;

    const gchar *name = gtk_widget_get_name (widget);

    /* Deal with what we are given from source */
    if ((selection_data != NULL) && (selection_data->length >= 0))
    {
        if (context-> action == GDK_ACTION_ASK)
        {
            /* Ask the user to move or copy, then set the context action. */
        }

        if (context-> action == GDK_ACTION_MOVE)
            delete_selection_data = TRUE;

        /* Check that we got the format we can use */
        switch (target_type)
        {
            case TARGET_STRING:
                _sdata = (gchar*)selection_data->data;

                Frame *frame = get_frame_view ();
                int rx = tbo_frame_get_base_x (x / zoom);
                int ry = tbo_frame_get_base_y (y / zoom);
                SVGImage *svgimage = tbo_svgimage_new_width_params (rx, ry, 0, 0, _sdata);
                update_drawing (tbo);
                tbo_frame_add_obj (frame, svgimage);

                dnd_success = TRUE;
                break;

            default:
                g_print ("nothing good");
        }
    }

    if (dnd_success == FALSE)
    {
        g_print ("DnD data transfer failed!\n");
    }

    gtk_drag_finish (context, dnd_success, delete_selection_data, time);
}

void
drag_data_get_handl (GtkWidget *widget,
                     GdkDragContext *context,
                     GtkSelectionData *selection_data,
                     guint target_type,
                     guint time,
                     char *svg)
{
    g_assert (selection_data != NULL);
    switch (target_type)
    {
        case TARGET_STRING:
            gtk_selection_data_set (selection_data,
                                    selection_data->target,
                                    8*sizeof(char),
                                    (guchar*) svg,
                                    strlen (svg));
            break;
        default:
            /* Default to some a safe target instead of fail. */
            g_assert_not_reached ();
    }
}

void
drag_begin_handl (GtkWidget *widget,
                  GdkDragContext *context,
                  char *svg)
{
    DND_IMAGE = gtk_image_new_from_file (svg);
    GdkPixbuf *pix = gtk_image_get_pixbuf (GTK_IMAGE (DND_IMAGE));
    gtk_drag_set_icon_pixbuf (context, pix, 0, 0);
}

void
drag_end_handl (GtkWidget *widget,
                GdkDragContext *context,
                gpointer user_data)
{
    if (DND_IMAGE)
    {
        gtk_widget_destroy (GTK_WIDGET (DND_IMAGE));
        DND_IMAGE = NULL;
    }
}
