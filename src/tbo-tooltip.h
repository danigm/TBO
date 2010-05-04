#ifndef __TBO_TOOLTIP_H__
#define __TBO_TOOLTIP_H__

#include <cairo.h>
#include <glib.h>
#include "tbo-window.h"

void tbo_tooltip_set (const char *tooltip, int x, int y, TboWindow *tbo);
void tbo_tooltip_set_center_timeout (const char *tooltip, int timeout, TboWindow *tbo);
GString *tbo_tooltip_get ();
void tbo_tooltip_draw (cairo_t *cr);

#endif
