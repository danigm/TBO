#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tbo-files.h"

char **tbo_files_get_dirs ()
{
    // Possible doodle dirs
    char **possible_dirs = malloc (3*sizeof(char*));
    possible_dirs[0] = malloc (255*sizeof(char*));
    possible_dirs[1] = malloc (255*sizeof(char*));
    possible_dirs[2] = NULL;

    strcat (strcpy (possible_dirs[0], getenv("HOME")), "/.tbo/doodle");
    strcpy (possible_dirs[1], DATA_DIR "/doodle");

    return possible_dirs;
}

int
tbo_files_prefix_len (char *str)
{
    int n, i = 0;
    char **possible_dirs = tbo_files_get_dirs ();
    while (possible_dirs[i])
    {
        if (g_str_has_prefix (str, possible_dirs[i]))
        {
            n = strlen (possible_dirs[i]) + 1;
            break;
        }
        i++;
    }
    tbo_files_free (possible_dirs);
    return n;
}

void
tbo_files_free (char **files)
{
    int i = 0;
    while(files[i])
    {
        free (files[i]);
        i++;
    }
    free (files);
}

void
tbo_files_expand_path (char *source, char *dest)
{
    int st, i = 0;
    char **possible_dirs = tbo_files_get_dirs ();
    struct stat filestat;
    while (possible_dirs[i])
    {
        snprintf (dest, 255, "%s/%s", possible_dirs[i], source);
        st = stat (dest, &filestat);
        if (!st)
            break;

        i++;
    }

    tbo_files_free (possible_dirs);
}
