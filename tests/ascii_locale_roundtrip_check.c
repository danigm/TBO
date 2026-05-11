#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <locale.h>
#include <math.h>
#include <string.h>

#include "comic.h"
#include "comic-load.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-text.h"
#include "tbo-window.h"

static gboolean
set_decimal_comma_locale (void)
{
    const char *locales[] = {
        "es_ES.utf8",
        "es_ES",
        "spanish",
    };
    guint i;

    for (i = 0; i < G_N_ELEMENTS (locales); i++)
    {
        if (setlocale (LC_NUMERIC, locales[i]) != NULL)
            return TRUE;
    }

    return FALSE;
}

int
main (int argc, char **argv)
{
    GtkApplication *app;
    TboWindow *tbo;
    gchar *tmpname;
    gint fd;
    gchar *contents = NULL;
    gsize length = 0;
    Page *page;
    Frame *frame;
    GdkRGBA frame_color;
    GdkRGBA text_color = {0.1, 0.2, 0.3, 1.0};
    TboObjectBase *obj;
    TboObjectText *text;
    Comic *reloaded;

    if (argc != 1)
        return 2;

    gtk_init ();

    if (!set_decimal_comma_locale ())
        return 77;

    app = gtk_application_new ("net.danigm.tbo.asciilocale", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 3;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 10, 20, 100, 80);
    tbo_frame_set_color_rgb (frame, 0.25, 0.5, 0.75);

    obj = TBO_OBJECT_BASE (tbo_object_text_new_with_params (5, 6, 70, 25, "hola", "Sans 12", &text_color));
    obj->angle = 1.25;
    tbo_frame_add_obj (frame, obj);

    tmpname = g_build_filename (g_get_tmp_dir (), "tbo-ascii-locale-XXXXXX.tbo", NULL);
    fd = g_mkstemp (tmpname);
    if (fd < 0)
        return 4;
    close (fd);

    if (!tbo_comic_save (tbo, tmpname))
        return 5;

    if (!g_file_get_contents (tmpname, &contents, &length, NULL))
        return 6;

    if (strstr (contents, "angle=\"1.25\"") == NULL)
        return 7;
    if (strstr (contents, "angle=\"1,25\"") != NULL)
        return 8;
    if (strstr (contents, "r=\"0.25\"") == NULL)
        return 9;
    if (strstr (contents, "r=\"0,25\"") != NULL)
        return 10;

    reloaded = tbo_comic_load (tmpname);
    if (reloaded == NULL)
        return 11;

    page = tbo_comic_get_current_page (reloaded);
    frame = tbo_page_get_frames (page)->data;
    tbo_frame_get_color (frame, &frame_color);
    if (fabs (frame_color.red - 0.25) > 1e-9 ||
        fabs (frame_color.green - 0.5) > 1e-9 ||
        fabs (frame_color.blue - 0.75) > 1e-9)
        return 12;

    obj = tbo_frame_get_objects (frame)->data;
    text = TBO_OBJECT_TEXT (obj);
    if (fabs (obj->angle - 1.25) > 1e-9)
        return 13;
    if (fabs (text->font_color->red - text_color.red) > 1e-6 ||
        fabs (text->font_color->green - text_color.green) > 1e-6 ||
        fabs (text->font_color->blue - text_color.blue) > 1e-6)
        return 14;

    g_free (contents);
    g_remove (tmpname);
    g_free (tmpname);
    tbo_comic_free (reloaded);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
