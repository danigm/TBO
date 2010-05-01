#ifndef __TBO_TOOLTIP_H__
#define __TBO_TOOLTIP_H__

#include <cairo.h>
#include <glib.h>

void tbo_tooltip_set (const char *tooltip, int x, int y);
GString *tbo_tooltip_get ();
void tbo_tooltip_draw (cairo_t *cr);

#endif
