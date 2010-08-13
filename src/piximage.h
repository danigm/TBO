/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __TBO_PIX_IMAGE__
#define __TBO_PIX_IMAGE__

#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"

typedef tbo_object PIXImage;

PIXImage * tbo_piximage_new ();
PIXImage * tbo_piximage_new_with_params (int x, int y, int width, int height, const char *path);
void tbo_pix_image_free (PIXImage *self);
void tbo_pix_image_draw (PIXImage *self, Frame *frame, cairo_t *cr);
void tbo_pix_image_save (PIXImage *self, FILE *file);
PIXImage * tbo_pix_image_clone (PIXImage *self);

#endif
