#ifndef __TBO_SVG_IMAGE__
#define __TBO_SVG_IMAGE__

#include <cairo.h>
#include "tbo-types.h"

typedef tbo_object SVGImage;

SVGImage * tbo_svgimage_new ();
SVGImage * tbo_svgimage_new_width_params (int x, int y, int width, int height, const char *path);
void tbo_svg_image_free (SVGImage *self);
void tbo_svg_image_draw (SVGImage *self, Frame *frame, cairo_t *cr);

#endif
