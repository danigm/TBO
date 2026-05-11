#include <gtk/gtk.h>

#include "comic.h"
#include "page.h"
#include "tbo-undo.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page1;
    Page *page2;
    Page *page3;
    GtkWidget *page_widget;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.pagereorder", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page1 = tbo_comic_get_current_page (tbo->comic);
    page2 = tbo_comic_new_page (tbo->comic);
    tbo_window_add_page_widget (tbo, create_darea (tbo), page2);
    page3 = tbo_comic_new_page (tbo->comic);
    tbo_window_add_page_widget (tbo, create_darea (tbo), page3);
    tbo_comic_set_current_page (tbo->comic, page1);
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_undo_stack_clear (tbo->undo_stack);

    page_widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tbo->notebook), 0);
    gtk_notebook_reorder_child (GTK_NOTEBOOK (tbo->notebook), page_widget, 2);

    if (tbo_comic_page_nth (tbo->comic, page1) != 2 ||
        tbo_comic_page_nth (tbo->comic, page2) != 0 ||
        tbo_comic_page_nth (tbo->comic, page3) != 1)
        return 3;
    if (g_object_get_data (G_OBJECT (gtk_notebook_get_nth_page (GTK_NOTEBOOK (tbo->notebook), 2)), "tbo-page") != page1)
        return 4;
    if (tbo_comic_get_current_page (tbo->comic) != page1 ||
        gtk_notebook_get_current_page (GTK_NOTEBOOK (tbo->notebook)) != 2)
        return 5;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_comic_page_nth (tbo->comic, page1) != 0 ||
        tbo_comic_page_nth (tbo->comic, page2) != 1 ||
        tbo_comic_page_nth (tbo->comic, page3) != 2)
        return 6;
    if (g_object_get_data (G_OBJECT (gtk_notebook_get_nth_page (GTK_NOTEBOOK (tbo->notebook), 0)), "tbo-page") != page1)
        return 7;

    tbo_window_redo_cb (NULL, tbo);
    if (tbo_comic_page_nth (tbo->comic, page1) != 2 ||
        g_object_get_data (G_OBJECT (gtk_notebook_get_nth_page (GTK_NOTEBOOK (tbo->notebook), 2)), "tbo-page") != page1)
        return 8;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
