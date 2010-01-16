#ifndef __TBO_PAGE__
#define __TBO_PAGE__

#include <gtk/gtk.h>
#include "tbo-types.h"

Page *tbo_page_new (Comic *comic);
void tbo_page_free (Page *page);
Frame *tbo_page_new_frame (Page *page, int x, int y, int w, int h);
void tbo_page_del_frame (Page *page, int nth);
int tbo_page_len (Page *page);
int tbo_page_frame_index (Page *page);
gboolean tbo_page_frame_first (Page *page);
gboolean tbo_page_frame_last (Page *page);
Frame *tbo_page_first_frame (Page *page);
Frame *tbo_page_next_frame (Page *page);
Frame *tbo_page_prev_frame (Page *page);
Frame *tbo_page_get_current_frame (Page *page);
void tbo_page_set_current_frame (Page *page, Frame *frame);
GList *tbo_page_get_frames (Page *page);

#endif

