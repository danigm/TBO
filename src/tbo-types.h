#ifndef __TBO_TYPES__
#define __TBO_TYPES__

#include <gtk/gtk.h>
#include <cairo.h>

typedef struct
{
    char *title;
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
    GList *objects;

} Frame;

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
    gpointer data;
};

typedef struct tbo_object tbo_object;

#endif

