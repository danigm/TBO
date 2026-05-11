#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <sys/stat.h>

#include "doodle-treeview.h"

GArray *get_files (gchar *base_dir, gboolean isdir, gboolean bubble_mode);
GtkWidget *doodle_add_images (gchar *dir);

int
main (void)
{
    gchar *dir;
    GtkWidget *grid;

    gtk_init ();

    dir = g_dir_make_tmp ("tbo-doodle-noaccess-XXXXXX", NULL);
    if (dir == NULL)
        return 2;

    if (g_chmod (dir, 0) != 0)
        return 3;

    if (get_files (dir, FALSE, FALSE) != NULL)
        return 4;

    grid = doodle_add_images (dir);
    if (!GTK_IS_GRID (grid))
        return 5;
    if (gtk_widget_get_first_child (grid) != NULL)
        return 6;

    g_chmod (dir, 0700);
    g_rmdir (dir);
    g_free (dir);
    return 0;
}
