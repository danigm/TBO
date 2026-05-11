#include <glib.h>
#include <string.h>

#include "comic-saveas-dialog.h"

int
main (void)
{
    gchar long_title[1024];
    gchar *filename;

    memset (long_title, 'a', sizeof (long_title) - 1);
    long_title[sizeof (long_title) - 1] = '\0';

    filename = tbo_comic_build_save_filename (long_title);
    if (filename == NULL)
        return 2;
    if (!g_str_has_suffix (filename, ".tbo"))
        return 3;
    if (strlen (filename) != strlen (long_title) + strlen (".tbo"))
        return 4;
    if (strncmp (filename, long_title, strlen (long_title)) != 0)
        return 5;
    g_free (filename);

    filename = tbo_comic_build_save_filename ("already.tbo");
    if (filename == NULL)
        return 6;
    if (strcmp (filename, "already.tbo") != 0)
        return 7;
    g_free (filename);

    return 0;
}
