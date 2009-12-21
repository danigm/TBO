#include <gtk/gtk.h>
#include <malloc.h>
#include "frame.h"

Frame *
tbo_frame_new (int width, int height)
{
    Frame *new_frame;

    new_frame = malloc (sizeof(Frame));
    new_frame->objects = NULL;

    new_frame->width = width;
    new_frame->height = height;

    return new_frame;
}

void 
tbo_frame_free (Frame *frame)
{
    // TODO free all frame objects
    g_list_free (frame->objects);
    free (frame);
}

