#ifndef __TBO_PAGE__
#define __TBO_PAGE__

#include <gtk/gtk.h>
#include "tbo-types.h"

Page *tbo_page_new (Comic *comic);
void tbo_page_free (Page *page);

#endif

