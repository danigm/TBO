#ifndef __TBO_SVG_IMAGE__
#define __TBO_SVG_IMAGE__

#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"

typedef tbo_object SVGImage;

SVGImage * tbo_svgimage_new ();
SVGImage * tbo_svgimage_new_with_params (int x, int y, int width, int height, const char *path);
void tbo_svg_image_free (SVGImage *self);
void tbo_svg_image_draw (SVGImage *self, Frame *frame, cairo_t *cr);
void tbo_svg_image_save (SVGImage *self, FILE *file);
SVGImage * tbo_svg_image_clone (SVGImage *self);

#endif
