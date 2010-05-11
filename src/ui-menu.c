#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "config.h"
#include "ui-menu.h"
#include "comic-new-dialog.h"
#include "comic-saveas-dialog.h"
#include "comic-open-dialog.h"
#include "tbo-window.h"
#include "ui-drawing.h"
#include "export.h"
#include "selector-tool.h"
#include "comic.h"
#include "frame.h"
#include "page.h"

static GtkActionGroup *MENU_ACTION_GROUP = NULL;

void
update_menubar (TboWindow *tbo)
{
    GtkAction *action;
    int i;
    char *actions[20] = {"FlipHObj", "FlipVObj", "OrderUpObj", "OrderDownObj", "DeleteObj", "CloneObj"};
    int elements = 6;
    int obj_n_elements = 4;
    gboolean activated = FALSE;

    tbo_object *obj = selector_tool_get_selected_obj ();
    Frame *frame = selector_tool_get_selected_frame ();

    if (!MENU_ACTION_GROUP)
        return;

    if (get_frame_view () && obj)
    {
        for (i=0; i<elements; i++)
        {
            action = gtk_action_group_get_action (MENU_ACTION_GROUP, actions[i]);
            gtk_action_set_sensitive (action, TRUE);
        }
    }
    else if (!get_frame_view () && frame)
    {
        for (i=obj_n_elements; i<elements; i++)
        {
            action = gtk_action_group_get_action (MENU_ACTION_GROUP, actions[i]);
            gtk_action_set_sensitive (action, TRUE);
        }
    }
    else
    {
        for (i=0; i<elements; i++)
        {
            action = gtk_action_group_get_action (MENU_ACTION_GROUP, actions[i]);
            gtk_action_set_sensitive (action, FALSE);
        }
    }
}

gboolean menu_handler (GtkWidget *widget, gpointer data){
    printf ("Menu :%s\n", ((TboWindow *) data)->comic->title);
    return FALSE;
}

gboolean
clone_obj_cb (GtkWidget *widget, gpointer data)
{
    tbo_object *obj = selector_tool_get_selected_obj ();
    Frame *frame = selector_tool_get_selected_frame ();
    Page *page = tbo_comic_get_current_page (((TboWindow*)data)->comic);

    if (!get_frame_view () && frame)
    {
        Frame *cloned_frame = tbo_frame_clone (frame);
        cloned_frame->x += 10;
        cloned_frame->y -= 10;
        tbo_page_add_frame (page, cloned_frame);
        set_selected (cloned_frame, (TboWindow*)data);
    }
    else if (obj && get_frame_view ())
    {
        tbo_object *cloned_obj = obj->clone (obj);
        cloned_obj->x += 10;
        cloned_obj->y -= 10;
        tbo_frame_add_obj (frame, cloned_obj);
        set_selected_obj (cloned_obj, (TboWindow*)data);
    }
    update_drawing ((TboWindow *)data);
    return FALSE;
}

gboolean
delete_obj_cb (GtkWidget *widget, gpointer data)
{
    TboWindow *tbo = (TboWindow *)data;

    tbo_object *obj = selector_tool_get_selected_obj ();
    Frame *frame = selector_tool_get_selected_frame ();
    Page *page = tbo_comic_get_current_page (((TboWindow*)data)->comic);

    if (obj && get_frame_view ())
    {
        tbo_frame_del_obj (frame, obj);
        set_selected_obj (NULL, tbo);
    }
    else if (!get_frame_view () && frame)
    {
        tbo_page_del_frame (page, frame);
        set_selected (NULL, tbo);
    }
    update_drawing ((TboWindow *)data);
    return FALSE;
}

gboolean
flip_v_cb (GtkWidget *widget, gpointer data)
{
    tbo_object *obj = selector_tool_get_selected_obj ();
    if (obj)
        tbo_object_flipv (obj);
    update_drawing ((TboWindow *)data);
    return FALSE;
}

gboolean
flip_h_cb (GtkWidget *widget, gpointer data)
{
    tbo_object *obj = selector_tool_get_selected_obj ();
    if (obj)
        tbo_object_fliph (obj);
    update_drawing ((TboWindow *)data);
    return FALSE;
}

gboolean
order_up_cb (GtkWidget *widget, gpointer data)
{
    tbo_object *obj = selector_tool_get_selected_obj ();
    if (obj)
        tbo_object_order_up (obj);
    update_drawing ((TboWindow *)data);
    return FALSE;
}

gboolean
order_down_cb (GtkWidget *widget, gpointer data)
{
    tbo_object *obj = selector_tool_get_selected_obj ();
    if (obj)
        tbo_object_order_down (obj);
    update_drawing ((TboWindow *)data);
    return FALSE;
}

gboolean close_cb (GtkWidget *widget, gpointer data){
    printf ("Ventana cerrada\n");
    tbo_window_free_cb (widget, NULL, ((TboWindow *) data));
    return FALSE;
}

gboolean
about_cb (GtkWidget *widget, TboWindow *tbo){
    const gchar *authors[] = {"danigm <dani@danigm.net>", NULL};
    gtk_show_about_dialog (GTK_WINDOW (tbo->window),
            "name", _("TBO comic editor"),
            "version", VERSION,
            "authors", authors,
            "website", "http://github.com/danigm/tbo",
            NULL);

    return FALSE;
}

gboolean
tbo_menu_to_png (GtkWidget *widget, TboWindow *tbo)
{
    tbo_export (tbo, "png");
    return FALSE;
}

gboolean
tbo_menu_to_pdf (GtkWidget *widget, TboWindow *tbo)
{
    tbo_export (tbo, "pdf");
    return FALSE;
}

gboolean
tbo_menu_to_svg (GtkWidget *widget, TboWindow *tbo)
{
    tbo_export (tbo, "svg");
    return FALSE;
}

static const GtkActionEntry tbo_menu_entries [] = {
    /* Toplevel */

    { "File", NULL, N_("_File") },
    { "Edit", NULL, N_("_Edit") },
    { "Help", NULL, N_("Help") },

    /* File menu */

    { "NewFile", GTK_STOCK_NEW, N_("_New"), "<control>N",
      N_("Create a new file"),
      G_CALLBACK (tbo_comic_new_dialog) },

    { "OpenFile", GTK_STOCK_OPEN, N_("_Open"), "<control>O",
      N_("Open a new file"),
      G_CALLBACK (tbo_comic_open_dialog) },

    { "SaveFile", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
      N_("Save current document"),
      G_CALLBACK (tbo_comic_save_dialog) },

    { "SaveFileAs", GTK_STOCK_SAVE_AS, N_("_Save as"), "",
      N_("Save current document as ..."),
      G_CALLBACK (tbo_comic_saveas_dialog) },

    { "ToPNG", GTK_STOCK_FILE, N_("Export as png"), "",
      N_("Save current document as png"),
      G_CALLBACK (tbo_menu_to_png) },

    { "ToPDF", GTK_STOCK_FILE, N_("Export as pdf"), "",
      N_("Save current document as pdf"),
      G_CALLBACK (tbo_menu_to_pdf) },

    { "ToSVG", GTK_STOCK_FILE, N_("Export as svg"), "",
      N_("Save current document as svg"),
      G_CALLBACK (tbo_menu_to_svg) },

    { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
      N_("Quit"),
      G_CALLBACK (close_cb) },

    /* edit menu */

    { "CloneObj", GTK_STOCK_COPY, N_("Clone"), "<control>d",
      N_("Clone"),
      G_CALLBACK (clone_obj_cb) },
    { "DeleteObj", GTK_STOCK_DELETE, N_("Delete"), "Delete",
      N_("Delete"),
      G_CALLBACK (delete_obj_cb) },
    { "FlipHObj", NULL, N_("Horizontal flip"), "h",
      N_("Horizontal flip"),
      G_CALLBACK (flip_h_cb) },
    { "FlipVObj", NULL, N_("Vertical flip"), "v",
      N_("Vertical flip"),
      G_CALLBACK (flip_v_cb) },
    { "OrderUpObj", NULL, N_("Move to front"), "Page_Up",
      N_("Move to front"),
      G_CALLBACK ( order_up_cb ) },
    { "OrderDownObj", NULL, N_("Move to back"), "Page_Down",
      N_("Move to back"),
      G_CALLBACK ( order_down_cb ) },

    /* Help menu */

    { "About", GTK_STOCK_ABOUT, N_("About"), "",
      N_("About"),
      G_CALLBACK (about_cb) },
};

GtkWidget *generate_menu (TboWindow *window){
    GtkWidget *menu;
    GtkUIManager *manager;
    GError *error = NULL;

    manager = gtk_ui_manager_new ();
    gtk_ui_manager_add_ui_from_file (manager, DATA_DIR "/ui/tbo-menu-ui.xml", &error);
    if (error != NULL)
    {
        g_warning (_("Could not merge tbo-menu-ui.xml: %s"), error->message);
        g_error_free (error);
    }

    MENU_ACTION_GROUP = gtk_action_group_new ("MenuActions");
    gtk_action_group_set_translation_domain (MENU_ACTION_GROUP, NULL);
    gtk_action_group_add_actions (MENU_ACTION_GROUP, tbo_menu_entries,
                        G_N_ELEMENTS (tbo_menu_entries), window);

    gtk_ui_manager_insert_action_group (manager, MENU_ACTION_GROUP, 0);

    menu = gtk_ui_manager_get_widget (manager, "/menubar");

    return menu;
}

