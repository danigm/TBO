#include <gtk/gtk.h>
#include <cairo.h>
#include "ui-drawing.h"
#include "tbo-types.h"
#include "tbo-object.h"

void
tbo_object_flipv (tbo_object *self)
{
    if (self->flipv)
        self->flipv = FALSE;
    else
        self->flipv = TRUE;
}

void
tbo_object_fliph (tbo_object *self)
{
    if (self->fliph)
        self->fliph = FALSE;
    else
        self->fliph = TRUE;
}

void
tbo_object_get_flip_matrix (tbo_object *self, cairo_matrix_t *mx)
{
    cairo_matrix_t flipv = {1, 0, 0, 1, 0, 0};
    cairo_matrix_t fliph = {1, 0, 0, 1, 0, 0};

    if (self->flipv)
    {
        cairo_matrix_init (&flipv, 1, 0, 0, -1, 0, self->height);
    }
    if (self->fliph)
    {
        cairo_matrix_init (&fliph, -1, 0, 0, 1, self->width, 0);
    }

    cairo_matrix_multiply (mx, &flipv, &fliph);
}

void
tbo_object_order_down (tbo_object *self)
{
    Frame *frame = get_frame_view ();
    GList *list = g_list_find (frame->objects, self);
    GList *prev = g_list_previous (list);
    tbo_object *tmp;
    if (prev)
    {
        tmp = (tbo_object*)list->data;
        list->data = prev->data;
        prev->data = tmp;
    }
}

void
tbo_object_order_up (tbo_object *self)
{
    Frame *frame = get_frame_view ();
    GList *list = g_list_find (frame->objects, self);
    GList *next = g_list_next (list);
    tbo_object *tmp;
    if (next)
    {
        tmp = (tbo_object*)list->data;
        list->data = next->data;
        next->data = tmp;
    }
}

void
tbo_object_move (tbo_object *self, enum MOVE_OPT type)
{
    switch (type)
    {
        case MOVE_UP:
            self->y -= MOVING_OFFSET;
            break;
        case MOVE_DOWN:
            self->y += MOVING_OFFSET;
            break;
        case MOVE_LEFT:
            self->x -= MOVING_OFFSET;
            break;
        case MOVE_RIGHT:
            self->x += MOVING_OFFSET;
            break;
        default:
            break;
    }
}
