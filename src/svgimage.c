#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include "svgimage.h"
#include "tbo-types.h"

#define DOODLE_DIR DATA_DIR "/doodle/"


SVGImage *
tbo_svgimage_new ()
{
    SVGImage *image;
    image = malloc (sizeof(SVGImage));
    image->data = malloc(sizeof(char)*255);
    image->free = tbo_svg_image_free;
    image->draw = tbo_svg_image_draw;
    image->save = tbo_svg_image_save;
    image->clone = tbo_svg_image_clone;
    image->type = SVGOBJ;
    image->flipv = FALSE;
    image->fliph = FALSE;
    return image;
}

SVGImage *
tbo_svgimage_new_with_params (int x,
                               int y,
                               int width,
                               int height,
                               const char *path)
{
    SVGImage *image;
    image = tbo_svgimage_new ();
    image->x = x;
    image->y = y;
    image->width = width;
    image->height = height;
    snprintf (image->data, 255, "%s", path);
    image->type = SVGOBJ;
    image->flipv = FALSE;
    image->fliph = FALSE;
    return image;
}

void
tbo_svg_image_free (SVGImage *self)
{
    free (self->data);
    free (self);
}

void
tbo_svg_image_draw (SVGImage *self, Frame *frame, cairo_t *cr)
{
    GError *error = NULL;
    RsvgHandle *rsvg_handle = NULL;
    RsvgDimensionData rsvg_dimension_data;
    char path[255];
    snprintf (path, 255, DOODLE_DIR "%s", self->data);
    rsvg_handle = rsvg_handle_new_from_file (path, &error);
    if (!rsvg_handle)
    {
        g_print (_("Couldn't load %s\n"), path);
        return;
    }
    if (error != NULL)
    {
        g_print ("%s\n", error->message);
        g_error_free (error);
        return;
    }
    else
    {
        rsvg_handle_get_dimensions (rsvg_handle, &rsvg_dimension_data);
        int w = rsvg_dimension_data.width;
        int h = rsvg_dimension_data.height;
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

        rsvg_handle_render_cairo (rsvg_handle, cr);

        cairo_scale (cr, 1/factorw, 1/factorh);
        cairo_transform (cr, &mx);
        cairo_rotate (cr, -self->angle);
        cairo_translate (cr, -(frame->x+self->x), -(frame->y+self->y));
        cairo_reset_clip (cr);

        g_object_unref (rsvg_handle);
    }
}

void
tbo_svg_image_save (SVGImage *self, FILE *file)
{
    char buffer[1024];

    snprintf (buffer, 1024, "   <svgimage x=\"%d\" y=\"%d\" "
                           "width=\"%d\" height=\"%d\" "
                           "angle=\"%f\" flipv=\"%d\" fliph=\"%d\" "
                           "path=\"%s\">\n ",
                           self->x, self->y, self->width, self->height,
                           self->angle, self->flipv, self->fliph, (char*)self->data);
    fwrite (buffer, sizeof (char), strlen (buffer), file);

    snprintf (buffer, 1024, "   </svgimage>\n");
    fwrite (buffer, sizeof (char), strlen (buffer), file);
}

SVGImage *
tbo_svg_image_clone (SVGImage *self)
{
    SVGImage *newimage;

    newimage = tbo_svgimage_new_with_params (self->x,
                                             self->y,
                                             self->width,
                                             self->height,
                                             self->data);
    newimage->angle = self->angle;
    newimage->flipv = self->flipv;
    newimage->fliph = self->fliph;

    return newimage;
}
