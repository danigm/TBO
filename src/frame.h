#ifndef __TBO_FRAME__
#define __TBO_FRAME__

#include <gtk/gtk.h>
#include "tbo-types.h"

Frame *tbo_frame_new (int witdh, int heigth);
void tbo_frame_free (Frame *frame);

#endif

