#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <string.h>
#include <unistd.h>

#include "comic-load.h"
#include "comic.h"

int
main (int argc, char **argv)
{
    gchar *tmpname;
    gint fd;
    Comic *comic;
    const gchar *invalid =
        "<tbo width=\"800\" height=\"450\">"
        " <page>"
        "  <frame x=\"0\" y=\"0\" width=\"100\" height=\"100\">"
        "   <text x=\"1\" y=\"2\" width=\"10\" height=\"10\">broken</text>"
        "  </frame>"
        " </page>"
        "</tbo>";

    if (argc != 2)
        return 2;

    gtk_init ();

    tmpname = g_build_filename (g_get_tmp_dir (), "tbo-invalid-XXXXXX.tbo", NULL);
    fd = g_mkstemp (tmpname);
    if (fd < 0)
        return 3;
    if (write (fd, invalid, strlen (invalid)) < 0)
        return 4;
    close (fd);

    comic = tbo_comic_load_with_alerts (tmpname, FALSE);
    g_remove (tmpname);
    g_free (tmpname);
    if (comic != NULL)
    {
        tbo_comic_free (comic);
        return 5;
    }

    comic = tbo_comic_load (argv[1]);
    if (comic == NULL)
        return 6;

    tbo_comic_free (comic);
    return 0;
}
