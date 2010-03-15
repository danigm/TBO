#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include "svgimage.h"
#include "ui-drawing.h"
#include "frame.h"
#include "doodle-treeview.h"
#include "dnd.h"
#include "tbo-utils.h"

void free_gstring_array (GArray *arr);

static GArray *TO_FREE = NULL;
static TboWindow *TBO = NULL;

void
doodle_free_all ()
{
    int i;
    if (!TO_FREE) return;
    for (i=0; i<TO_FREE->len; i++)
    {
        free_gstring_array (g_array_index (TO_FREE, GArray*, i));
    }
    g_array_free (TO_FREE, TRUE);
    TO_FREE = NULL;
}

void doodle_add_to_free (GArray *arr)
{
    if (!TO_FREE)
        TO_FREE = g_array_new (FALSE, FALSE, sizeof(GArray*));

    g_array_append_val (TO_FREE, arr);
}

gboolean
on_doodle_click_cb (GtkWidget      *widget,
                    GdkEventButton *event,
                    gpointer       *data)
{
    Frame *frame = get_frame_view ();
    SVGImage *svgimage = tbo_svgimage_new_width_params (0, 0, 0, 0, (char*)data);
    update_drawing (TBO);
    tbo_frame_add_obj (frame, svgimage);
}

void
free_gstring_array (GArray *arr)
{
    int i;
    GString *mystr;

    for (i=0; i<arr->len; i++)
    {
        mystr = g_array_index (arr, GString*, i);
        g_string_free (mystr, TRUE);
    }
    g_array_free (arr, TRUE);
}

GArray *
get_files (gchar *base_dir, gboolean isdir)
{
    GError *error = NULL;
    GDir *dir = g_dir_open (base_dir, 0, &error);
    gchar complete_dir[255];
    const gchar *filename;
    struct stat filestat;
    GArray *array = g_array_new (FALSE, FALSE, sizeof(GString*));

    while (filename = g_dir_read_name (dir))
    {
        size_t strsize = sizeof (char) * (strlen (base_dir) + strlen (filename) + 2);
        snprintf (complete_dir, strsize, "%s/%s", base_dir, filename);
        stat (complete_dir, &filestat);

        if (isdir && S_ISDIR (filestat.st_mode))
        {
            GString *dirname_to_append = g_string_new (complete_dir);
            g_array_append_val (array, dirname_to_append);
        }
        else if (!isdir && !S_ISDIR (filestat.st_mode))
        {
            GString *filename_to_append = g_string_new (complete_dir);
            g_array_append_val (array, filename_to_append);
        }
    }

    g_dir_close (dir);

    return array;
}

GtkWidget *
doodle_add_images (gchar *dir)
{
    int i;
    gchar *dirname;
    GtkWidget *table;
    GtkWidget *image;
    GtkWidget *ebox;
    GdkPixbuf *pixbuf;
    int r, c=2;
    int left, top;
    int w=50, h;

    dirname = dir;

    GArray *arr = get_files (dirname, FALSE);

    r = (arr->len / c) + 1;
    table = gtk_table_new (r, c, TRUE);

    GString *mystr;
    for (i=0; i<arr->len; i++)
    {
        top = i / 2;
        left = i % 2;

        mystr = g_array_index (arr, GString*, i);
        image = gtk_image_new_from_file (mystr->str);
        pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (image));

        h = gdk_pixbuf_get_height (pixbuf) * 50 / (float)gdk_pixbuf_get_width (pixbuf);
        pixbuf = gdk_pixbuf_scale_simple (pixbuf, w, h, GDK_INTERP_BILINEAR);

        gtk_widget_destroy (GTK_WIDGET (image));
        image = gtk_image_new_from_pixbuf (pixbuf);
        ebox = gtk_event_box_new ();
        gtk_widget_add_events (ebox, GDK_BUTTON_PRESS_MASK |
                                     GDK_BUTTON_RELEASE_MASK |
                                     GDK_POINTER_MOTION_MASK);

        //g_signal_connect (ebox, "button_press_event", G_CALLBACK (on_doodle_click_cb), mystr->str);

        // dnd
        gtk_drag_source_set (ebox,
                             GDK_BUTTON1_MASK,
                             TARGET_LIST,
                             N_TARGETS,
                             GDK_ACTION_COPY);
        g_signal_connect (ebox, "drag-data-get", G_CALLBACK (drag_data_get_handl),
                                                mystr->str + strlen (DATA_DIR "/doodle/"));
        g_signal_connect (ebox, "drag-begin", G_CALLBACK (drag_begin_handl), mystr->str);
        g_signal_connect (ebox, "drag-end", G_CALLBACK (drag_end_handl), mystr->str);

        gtk_container_add (GTK_CONTAINER (ebox), image);
        gtk_table_attach_defaults (GTK_TABLE (table), ebox, left, left + 1, top, top + 1);
    }

    doodle_add_to_free (arr);

    gtk_widget_show_all (GTK_WIDGET (table));
    return table;
}

void
doodle_add_dir_images (gchar *dir, GtkWidget *box)
{
    char base_name[255];
    get_base_name (dir, base_name, 255);
    GtkWidget *expander = gtk_expander_new (base_name);
    GtkWidget *table = doodle_add_images (dir);
    gtk_container_add (GTK_CONTAINER (expander), table);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
    gtk_container_add (GTK_CONTAINER (box), expander);
}

void
on_expand_cb (GtkExpander *expander, GString *str)
{
    GString *mystr2;
    int i;
    GtkWidget *vbox = g_list_first (gtk_container_get_children (GTK_CONTAINER (expander)))->data;
    int numofchilds = g_list_length (gtk_container_get_children (GTK_CONTAINER (vbox)));
    if (numofchilds == 0)
    {
        GArray *subdir = get_files (str->str, TRUE);
        for (i=0; i<subdir->len; i++)
        {
            mystr2 = g_array_index (subdir, GString*, i);
            doodle_add_dir_images (mystr2->str, vbox);
        }
        free_gstring_array (subdir);
        g_string_free (str, TRUE);
    }
    gtk_widget_show_all (GTK_WIDGET (vbox));
}

GtkWidget *
doodle_setup_tree (TboWindow *tbo)
{
    GtkWidget *expander;
    GtkWidget *vbox;
    GtkWidget *vbox2;
    gchar *dirname;

    TBO = tbo;

    dirname = malloc (255*sizeof(char));
    char label_format[255];

    vbox = gtk_vbox_new (FALSE, 5);

    GArray *arr = get_files (DATA_DIR "/doodle", TRUE);
    int i, j;
    GString *mystr, *mystr2;
    for (i=0; i<arr->len; i++)
    {
        mystr = g_array_index (arr, GString*, i);

        vbox2 = gtk_vbox_new (FALSE, 5);
        get_base_name (mystr->str, dirname, 255);
        snprintf (label_format, 255, "<span underline=\"single\" size=\"large\" weight=\"ultrabold\">%s</span>", dirname);
        expander = gtk_expander_new (label_format);
        gtk_expander_set_use_markup (GTK_EXPANDER (expander), TRUE);
        gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 5);
        gtk_container_add (GTK_CONTAINER (expander), vbox2);

        mystr2 = g_string_new (mystr->str);
        g_signal_connect (GTK_EXPANDER (expander), "activate", G_CALLBACK (on_expand_cb), mystr2);
    }
    free_gstring_array (arr);

    free (dirname);

    return vbox;
}
