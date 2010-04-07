#ifndef __TBO_PIX_IMAGE__
#define __TBO_PIX_IMAGE__

#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"

typedef tbo_object PIXImage;

PIXImage * tbo_piximage_new ();
PIXImage * tbo_piximage_new_width_params (int x, int y, int width, int height, const char *path);
void tbo_pix_image_free (PIXImage *self);
void tbo_pix_image_draw (PIXImage *self, Frame *frame, cairo_t *cr);
void tbo_pix_image_save (PIXImage *self, FILE *file);
PIXImage * tbo_pix_image_clone (PIXImage *self);

#endif
