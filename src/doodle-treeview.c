#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include "svgimage.h"
#include "ui-drawing.h"
#include "frame.h"
#include "doodle-treeview.h"
#include "dnd.h"

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

void
get_base_name (gchar *str, gchar *ret, int size)
{
    gchar **paths;
    gchar **dirname;
    paths = g_strsplit (str, "/", 0);
    dirname = paths;
    while (*dirname) dirname++;
    dirname--;
    snprintf (ret, size, *dirname);
    g_strfreev (paths);
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
doodle_add_images (gchar *dir, gchar *subdir)
{
    int i;
    gchar *dirname;
    GtkWidget *table;
    GtkWidget *image;
    GtkWidget *ebox;
    GdkPixbuf *pixbuf;
    int r, c=2;
    int left, top;
    int w, h=50;

    dirname = malloc (255*sizeof(char));
    snprintf (dirname, 255, "%s/%s", dir, subdir);

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

        w = gdk_pixbuf_get_width (pixbuf) * 50 / (float)gdk_pixbuf_get_height (pixbuf);
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
        g_signal_connect (ebox, "drag-data-get", G_CALLBACK (drag_data_get_handl), mystr->str);
        g_signal_connect (ebox, "drag-begin", G_CALLBACK (drag_begin_handl), mystr->str);
        g_signal_connect (ebox, "drag-end", G_CALLBACK (drag_end_handl), mystr->str);

        gtk_container_add (GTK_CONTAINER (ebox), image);
        gtk_table_attach_defaults (GTK_TABLE (table), ebox, left, left + 1, top, top + 1);
    }

    doodle_add_to_free (arr);
    free (dirname);

    gtk_widget_show_all (GTK_WIDGET (table));
    return table;
}

void
doodle_add_body_images (gchar *dir, GtkWidget *box)
{
    GtkWidget *expander = gtk_expander_new ("body");
    GtkWidget *table = doodle_add_images (dir, "body");
    gtk_container_add (GTK_CONTAINER (expander), table);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
    gtk_container_add (GTK_CONTAINER (box), expander);
}

void
doodle_add_eyes_images (gchar *dir, GtkWidget *box)
{
    GtkWidget *expander = gtk_expander_new ("eyes");
    GtkWidget *table = doodle_add_images (dir, "eyes");
    gtk_container_add (GTK_CONTAINER (expander), table);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
    gtk_container_add (GTK_CONTAINER (box), expander);
}

void
doodle_add_mouth_images (gchar *dir, GtkWidget *box)
{
    GtkWidget *expander = gtk_expander_new ("mouth");
    GtkWidget *table = doodle_add_images (dir, "mouth");
    gtk_container_add (GTK_CONTAINER (expander), table);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
    gtk_container_add (GTK_CONTAINER (box), expander);
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

    vbox = gtk_vbox_new (FALSE, 5);
    vbox2 = gtk_vbox_new (FALSE, 5);

    GArray *arr = get_files (DATA_DIR "/doodle", TRUE);
    int i;
    GString *mystr;
    for (i=0; i<arr->len; i++)
    {
        mystr = g_array_index (arr, GString*, i);

        get_base_name (mystr->str, dirname, 255);
        expander = gtk_expander_new (dirname);
        gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 5);
        gtk_container_add (GTK_CONTAINER (expander), vbox2);

        doodle_add_body_images (mystr->str, vbox2);
        doodle_add_eyes_images (mystr->str, vbox2);
        doodle_add_mouth_images (mystr->str, vbox2);
    }
    free_gstring_array (arr);

    free (dirname);

    return vbox;
}
