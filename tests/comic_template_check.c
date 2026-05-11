#include <gtk/gtk.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-window.h"

static gboolean
page_frames_are_valid (Page *page, gint width, gint height)
{
    GList *frames;

    for (frames = tbo_page_get_frames (page); frames != NULL; frames = frames->next)
    {
        Frame *frame = frames->data;
        gint x;
        gint y;
        gint frame_width;
        gint frame_height;

        tbo_frame_get_bounds (frame, &x, &y, &frame_width, &frame_height);
        if (frame_width <= 0 || frame_height <= 0)
            return FALSE;
        if (x < 0 || y < 0)
            return FALSE;
        if (x + frame_width > width || y + frame_height > height)
            return FALSE;
    }

    return TRUE;
}

static gint
expected_frame_count (TboComicTemplate template)
{
    switch (template)
    {
        case TBO_COMIC_TEMPLATE_STRIP:
            return 3;
        case TBO_COMIC_TEMPLATE_A4:
            return 6;
        case TBO_COMIC_TEMPLATE_STORYBOARD:
            return 4;
        case TBO_COMIC_TEMPLATE_EMPTY:
        case TBO_COMIC_TEMPLATE_N_TEMPLATES:
        default:
            return 0;
    }
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    gint width;
    gint height;
    guint template;

    gtk_init ();

    tbo_comic_template_get_default_size (TBO_COMIC_TEMPLATE_EMPTY, &width, &height);
    if (width != 800 || height != 500)
        return 2;
    tbo_comic_template_get_default_size (TBO_COMIC_TEMPLATE_STRIP, &width, &height);
    if (width != 1800 || height != 600)
        return 3;
    tbo_comic_template_get_default_size (TBO_COMIC_TEMPLATE_A4, &width, &height);
    if (width != 1240 || height != 1754)
        return 4;
    tbo_comic_template_get_default_size (TBO_COMIC_TEMPLATE_STORYBOARD, &width, &height);
    if (width != 1600 || height != 900)
        return 5;

    app = gtk_application_new ("net.danigm.tbo.comictemplate", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 6;

    for (template = TBO_COMIC_TEMPLATE_EMPTY; template < TBO_COMIC_TEMPLATE_N_TEMPLATES; template++)
    {
        tbo_comic_template_get_default_size (template, &width, &height);
        tbo = tbo_new_tbo_with_template (app, width, height, template);
        page = tbo_comic_get_current_page (tbo->comic);

        if (tbo_comic_get_width (tbo->comic) != width || tbo_comic_get_height (tbo->comic) != height)
            return 10 + template;
        if (page == NULL)
            return 20 + template;
        if (tbo_page_len (page) != expected_frame_count (template))
            return 30 + template;
        if ((template == TBO_COMIC_TEMPLATE_A4 && tbo_comic_get_paper (tbo->comic) != TBO_COMIC_PAPER_A4) ||
            (template != TBO_COMIC_TEMPLATE_A4 && tbo_comic_get_paper (tbo->comic) != TBO_COMIC_PAPER_NONE))
            return 35 + template;
        if (!page_frames_are_valid (page, width, height))
            return 40 + template;

        tbo_window_apply_comic_template (tbo, TBO_COMIC_TEMPLATE_STRIP);
        if (tbo_page_len (page) != 3)
            return 50 + template;
        tbo_window_apply_comic_template (tbo, TBO_COMIC_TEMPLATE_EMPTY);
        if (tbo_page_len (page) != 0)
            return 60 + template;

        tbo_window_mark_clean (tbo);
        gtk_window_close (GTK_WINDOW (tbo->window));
        while (g_main_context_iteration (NULL, FALSE));
    }

    g_object_unref (app);
    return 0;
}
