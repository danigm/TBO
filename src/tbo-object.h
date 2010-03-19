#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"

enum MOVE_OPT
{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
};

enum RESIZE_OPT
{
    RESIZE_LESS,
    RESIZE_GREATER,
};

static int MOVING_OFFSET = 10;

void tbo_object_flipv (tbo_object *self);
void tbo_object_fliph (tbo_object *self);
void tbo_object_get_flip_matrix (tbo_object *self, cairo_matrix_t *mx);
void tbo_object_order_down (tbo_object *self);
void tbo_object_order_up (tbo_object *self);
void tbo_object_move (tbo_object *self, enum MOVE_OPT type);
void tbo_object_resize (tbo_object *self, enum RESIZE_OPT type);
void tbo_object_save (tbo_object *self, FILE *file);
