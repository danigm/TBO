#ifndef __TBO_TEXT_OBJ__
#define __TBO_TEXT_OBJ__

#include <cairo.h>
#include "tbo-types.h"

typedef tbo_object TextObj;

TextObj * tbo_text_new ();
TextObj * tbo_text_new_width_params (int x, int y, int width, int height, const char *text, char *font_name, double r, double g, double b);
void tbo_text_free (TextObj *self);
void tbo_text_draw (TextObj *self, Frame *frame, cairo_t *cr);
char *tbo_text_get_text (TextObj *self);
void tbo_text_set_text (TextObj *self, const char *text);
void tbo_text_change_font (TextObj *self, char *font);
void tbo_text_change_color (TextObj *self, double r, double g, double b);

#endif
