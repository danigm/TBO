#ifndef __TBO_TYPES__
#define __TBO_TYPES__

#include <gtk/gtk.h>

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

#endif

