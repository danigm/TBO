#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <string.h>
#include <unistd.h>

#include "comic-load.h"
#include "comic.h"

static gboolean
invalid_case_fails (const gchar *xml)
{
    gchar *tmpname = g_build_filename (g_get_tmp_dir (), "tbo-invalid-variant-XXXXXX.tbo", NULL);
    gint fd = g_mkstemp (tmpname);
    Comic *comic;

    if (fd < 0)
        return FALSE;
    if (write (fd, xml, strlen (xml)) < 0)
    {
        close (fd);
        g_remove (tmpname);
        g_free (tmpname);
        return FALSE;
    }
    close (fd);

    comic = tbo_comic_load_with_alerts (tmpname, FALSE);
    g_remove (tmpname);
    g_free (tmpname);
    if (comic != NULL)
    {
        tbo_comic_free (comic);
        return FALSE;
    }

    return TRUE;
}

int
main (int argc, char **argv)
{
    const gchar *cases[] = {
        "<tbo width=\"800\" height=\"450\"><page><frame x=\"0\" y=\"0\" width=\"100\" height=\"100\"><svgimage x=\"0\" y=\"0\" width=\"10\" height=\"10\"/></frame></page></tbo>",
        "<tbo width=\"800\" height=\"450\"><page><frame x=\"0\" y=\"0\" width=\"-10\" height=\"100\"/></page></tbo>",
        "<tbo width=\"800\" height=\"450\"><frame x=\"0\" y=\"0\" width=\"100\" height=\"100\"/></tbo>",
        "<tbo width=\"800\" height=\"450\"><page><text x=\"1\" y=\"2\" width=\"10\" height=\"10\" font=\"Sans 12\">broken</text></page></tbo>",
        "<tbo width=\"800\" height=\"450\"></tbo><tbo width=\"1\" height=\"1\"></tbo>",
        "<tbo width=\"800\" height=\"450\"></tbo>",
        "<tbo width=\"800\" height=\"450\"><page><frame x=\"0\" y=\"0\" width=\"100\" height=\"100\"><svgimage x=\"0\" y=\"0\" width=\"-1\" height=\"10\" path=\"foo.svg\"></svgimage></frame></page></tbo>",
        "<tbo width=\"800\" height=\"450\"><page><frame x=\"0\" y=\"0\" width=\"100\" height=\"100\"><piximage x=\"0\" y=\"0\" width=\"10\" height=\"-1\" path=\"foo.png\"></piximage></frame></page></tbo>",
        "<tbo width=\"800\" height=\"450\"><page><frame x=\"0\" y=\"0\" width=\"100\" height=\"100\"><text x=\"0\" y=\"0\" width=\"-1\" height=\"0\" font=\"Sans 12\">text</text></frame></page></tbo>",
    };
    guint i;
    Comic *comic;

    if (argc != 2)
        return 2;

    gtk_init ();

    for (i = 0; i < G_N_ELEMENTS (cases); i++)
    {
        if (!invalid_case_fails (cases[i]))
            return 3 + i;
    }

    comic = tbo_comic_load (argv[1]);
    if (comic == NULL)
        return 10;
    tbo_comic_free (comic);
    return 0;
}
