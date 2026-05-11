#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>

#include "tbo-utils.h"

int
main (void)
{
    const gchar *translated;
    const gchar *translated_status;
    gchar *locale_dir;

    g_setenv ("LC_ALL", "es_ES.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "es_ES:es", TRUE);

    tbo_init_i18n ();

    locale_dir = tbo_get_locale_path ();
    if (g_file_test (locale_dir, G_FILE_TEST_IS_DIR) == FALSE)
        return 2;

    translated = _("Untitled");
    g_free (locale_dir);

    if (strcmp (translated, "Untitled") == 0)
        return 3;
    if (g_str_has_prefix (translated, "Sin") == FALSE)
        return 4;

    translated_status = _("Page %d of %d");
    if (strcmp (translated_status, "Page %d of %d") == 0)
        return 5;
    if (g_str_has_prefix (translated_status, "Pá") == FALSE)
        return 6;

    return 0;
}
