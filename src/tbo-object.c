#include <gtk/gtk.h>
#include <cairo.h>
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
    cairo_matrix_t flipv = {1, 0, 0, -1, 0, 0};
    cairo_matrix_t fliph = {-1, 0, 0, 1, 0, 0};

    if (self->flipv)
    {
        mx->xx = flipv.xx; mx->yx = flipv.yx; mx->yy = flipv.yy;
        mx->x0 = 0;
        mx->y0 = self->height;
    }
    else if (self->fliph)
    {
        mx->xx = fliph.xx; mx->yx = fliph.yx; mx->yy = fliph.yy;
        mx->x0 = self->width;
        mx->y0 = 0;
    }
}
