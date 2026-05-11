#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <string.h>

#include "comic-load.h"
#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-pixmap.h"
#include "tbo-object-svg.h"
#include "tbo-object-text.h"
#include "tbo-window.h"

static gchar *
build_long_text (void)
{
    GString *text = g_string_new ("\n  Header <&> \"quoted\"\n");
    gint i;

    for (i = 0; i < 300; i++)
        g_string_append_printf (text, "Line %d <tag>&\"value\"\n", i);

    g_string_append (text, "Tail with spaces   \n");

    return g_string_free (text, FALSE);
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    Frame *frame;
    TboObjectText *text;
    TboObjectSvg *svg;
    TboObjectPixmap *pixmap;
    GdkRGBA color = { 0.1, 0.2, 0.3, 1.0 };
    gchar *long_text;
    gchar *expected_text;
    gchar *tmpname;
    gint fd;
    gchar *contents = NULL;
    Comic *reloaded;
    GList *objects;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.xmlroundtrip", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 0, 0, 300, 200);
    long_text = build_long_text ();
    expected_text = g_strdup (long_text);

    text = TBO_OBJECT_TEXT (tbo_object_text_new_with_params (10, 10, 200, 100, long_text, "Sans 12", &color));
    svg = TBO_OBJECT_SVG (tbo_object_svg_new_with_params (5, 5, 20, 20, "assets/& weird \"quote\" <svg>.svg"));
    pixmap = TBO_OBJECT_PIXMAP (tbo_object_pixmap_new_with_params (15, 15, 25, 25, "assets/& weird \"quote\" <pix>.png"));
    tbo_frame_add_obj (frame, TBO_OBJECT_BASE (text));
    tbo_frame_add_obj (frame, TBO_OBJECT_BASE (svg));
    tbo_frame_add_obj (frame, TBO_OBJECT_BASE (pixmap));

    tmpname = g_build_filename (g_get_tmp_dir (), "tbo-xml-roundtrip-XXXXXX.tbo", NULL);
    fd = g_mkstemp (tmpname);
    if (fd < 0)
        return 3;
    close (fd);

    if (!tbo_comic_save (tbo, tmpname))
        return 4;
    if (!g_file_get_contents (tmpname, &contents, NULL, NULL))
        return 5;
    if (strstr (contents, "assets/& weird") != NULL)
        return 6;
    if (strstr (contents, "&amp; weird &quot;quote&quot; &lt;svg&gt;.svg") == NULL)
        return 7;
    if (strstr (contents, "Header <&>") != NULL)
        return 8;
    if (strstr (contents, "Header &lt;&amp;&gt; &quot;quoted&quot;") == NULL)
        return 9;

    reloaded = tbo_comic_load (tmpname);
    if (reloaded == NULL)
        return 10;

    page = tbo_comic_get_current_page (reloaded);
    frame = tbo_page_get_frames (page)->data;
    text = NULL;
    svg = NULL;
    pixmap = NULL;
    for (objects = tbo_frame_get_objects (frame); objects != NULL; objects = objects->next)
    {
        if (TBO_IS_OBJECT_TEXT (objects->data))
            text = TBO_OBJECT_TEXT (objects->data);
        else if (TBO_IS_OBJECT_SVG (objects->data))
            svg = TBO_OBJECT_SVG (objects->data);
        else if (TBO_IS_OBJECT_PIXMAP (objects->data))
            pixmap = TBO_OBJECT_PIXMAP (objects->data);
    }

    if (text == NULL || svg == NULL || pixmap == NULL)
        return 11;
    if (strcmp (tbo_object_text_get_text (text), expected_text) != 0)
        return 12;
    if (strcmp (svg->path->str, "assets/& weird \"quote\" <svg>.svg") != 0)
        return 13;
    if (strcmp (pixmap->path->str, "assets/& weird \"quote\" <pix>.png") != 0)
        return 14;

    g_free (contents);
    g_remove (tmpname);
    g_free (tmpname);
    g_free (long_text);
    g_free (expected_text);
    tbo_comic_free (reloaded);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
