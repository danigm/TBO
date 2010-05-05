#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include "piximage.h"
#include "tbo-types.h"

#define DOODLE_DIR DATA_DIR "/doodle/"


PIXImage *
tbo_piximage_new ()
{
    PIXImage *image;
    image = malloc (sizeof(PIXImage));
    image->data = malloc(sizeof(char)*255);
    image->free = tbo_pix_image_free;
    image->draw = tbo_pix_image_draw;
    image->save = tbo_pix_image_save;
    image->clone = tbo_pix_image_clone;
    image->angle = 0.0;
    image->type = PIXOBJ;
    image->flipv = FALSE;
    image->fliph = FALSE;
    return image;
}

PIXImage *
tbo_piximage_new_with_params (int x,
                               int y,
                               int width,
                               int height,
                               const char *path)
{
    PIXImage *image;
    image = tbo_piximage_new ();
    image->x = x;
    image->y = y;
    image->width = width;
    image->height = height;
    snprintf (image->data, 255, "%s", path);
    image->type = PIXOBJ;
    image->flipv = FALSE;
    image->fliph = FALSE;
    return image;
}

void
tbo_pix_image_free (PIXImage *self)
{
    free (self->data);
    free (self);
}

void
tbo_pix_image_draw (PIXImage *self, Frame *frame, cairo_t *cr)
{
    int w, h;
    cairo_surface_t *image;

    image = cairo_image_surface_create_from_png (self->data);
    w = cairo_image_surface_get_width (image);
    h = cairo_image_surface_get_height (image);

    if (!self->width) self->width = w;
    if (!self->height) self->height = h;

    float factorw = (float)self->width / (float)w;
    float factorh = (float)self->height / (float)h;

    cairo_matrix_t mx = {1, 0, 0, 1, 0, 0};
    tbo_object_get_flip_matrix (self, &mx);

    cairo_rectangle(cr, frame->x+2, frame->y+2, frame->width-4, frame->height-4);
    cairo_clip (cr);
    cairo_translate (cr, frame->x+self->x, frame->y+self->y);
    cairo_rotate (cr, self->angle);
    cairo_transform (cr, &mx);
    cairo_scale (cr, factorw, factorh);

    cairo_set_source_surface (cr, image, 0, 0);
    cairo_paint (cr);

    cairo_scale (cr, 1/factorw, 1/factorh);
    cairo_transform (cr, &mx);
    cairo_rotate (cr, -self->angle);
    cairo_translate (cr, -(frame->x+self->x), -(frame->y+self->y));
    cairo_reset_clip (cr);


    cairo_surface_destroy (image);
}

void
tbo_pix_image_save (PIXImage *self, FILE *file)
{
    char buffer[1024];

    snprintf (buffer, 1024, "   <piximage x=\"%d\" y=\"%d\" "
                           "width=\"%d\" height=\"%d\" "
                           "angle=\"%f\" flipv=\"%d\" fliph=\"%d\" "
                           "path=\"%s\">\n ",
                           self->x, self->y, self->width, self->height,
                           self->angle, self->flipv, self->fliph, (char*)self->data);
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    snprintf (buffer, 1024, "   </piximage>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
}

PIXImage *
tbo_pix_image_clone (PIXImage *self)
{
    PIXImage *newimage;

    newimage = tbo_piximage_new_with_params (self->x,
                                              self->y,
                                              self->width,
                                              self->height,
                                              self->data);
    newimage->angle = self->angle;
    newimage->flipv = self->flipv;
    newimage->fliph = self->fliph;

    return newimage;
}
