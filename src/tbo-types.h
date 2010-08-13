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


#ifndef __TBO_TYPES__
#define __TBO_TYPES__

#include <gtk/gtk.h>
#include <stdio.h>
#include <cairo.h>

typedef struct
{
    double r;
    double g;
    double b;
} Color;

typedef struct
{
    char title[255];
    int width;
    int height;
    GList *pages;

} Comic;

typedef struct
{
    Comic *comic;
    GList *frames;

} Page;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
    gboolean border;
    Color *color;
    GList *objects;

} Frame;

enum TYPE
{
    SVGOBJ,
    PIXOBJ,
    TEXTOBJ,
};

struct tbo_object
{
    int x;
    int y;
    int width;
    int height;
    double angle;
    gboolean flipv;
    gboolean fliph;
    void (*free) (struct tbo_object *);
    void (*draw) (struct tbo_object *, Frame *, cairo_t *);
    void (*save) (struct tbo_object *, FILE *);
    struct tbo_object * (*clone) (struct tbo_object *);
    enum TYPE type;
    gpointer data;
};

typedef struct tbo_object tbo_object;

#endif

