/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2011 Daniel Garcia <danigm@wadobo.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify
 * it under the terms of the GNU General Public License as published
 * by
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

#ifndef __TBO_UNDO__
#define __TBO_UNDO__

#include <glib.h>
#include <stdlib.h>
#include "tbo-types.h"

typedef struct _TboObjectBase TboObjectBase;
typedef struct _TboObjectText TboObjectText;

#define tbo_action_new(action_type) (TboAction*) calloc (1, sizeof (action_type))
#define tbo_action_do(action) ((TboAction*) action)->action_do ((TboAction*)action)
#define tbo_action_undo(action) ((TboAction*) action)->action_undo ((TboAction*)action)


typedef struct _TboAction TboAction;
typedef struct _TboUndoStack TboUndoStack;

struct _TboAction {
    void (*action_do) (TboAction *action);
    void (*action_undo) (TboAction *action);
    void (*action_free) (TboAction *action);
    guint64 state_before;
    guint64 state_after;
};

void tbo_action_del (TboAction *action);

struct _TboUndoStack {
    GList *first;
    GList *list;
    gboolean last_flag;
    guint64 current_state_id;
    guint64 next_state_id;
};

TboUndoStack * tbo_undo_stack_new (void);
void tbo_undo_stack_del (TboUndoStack *stack);
void tbo_undo_stack_clear (TboUndoStack *stack);
void tbo_undo_stack_insert (TboUndoStack *stack, TboAction *action);
void tbo_undo_stack_undo (TboUndoStack *stack);
void tbo_undo_stack_redo (TboUndoStack *stack);

gboolean tbo_undo_active_undo (TboUndoStack *stack);
gboolean tbo_undo_active_redo (TboUndoStack *stack);

TboAction * tbo_action_frame_add_new (Page *page, Frame *frame);
TboAction * tbo_action_page_add_new (Comic *comic, Page *page, int index);
TboAction * tbo_action_page_remove_new (Comic *comic, Page *page, int index);
TboAction * tbo_action_page_reorder_new (Comic *comic, Page *page, int index1, int index2);
TboAction * tbo_action_frame_remove_new (Page *page, Frame *frame, int index);
TboAction * tbo_action_object_add_new (Frame *frame, TboObjectBase *object);
TboAction * tbo_action_object_remove_new (Frame *frame, TboObjectBase *object, int index);
TboAction * tbo_action_frame_state_new (Frame *frame,
                                        int x1, int y1, int width1, int height1,
                                        gboolean border1, gdouble r1, gdouble g1, gdouble b1,
                                        int x2, int y2, int width2, int height2,
                                        gboolean border2, gdouble r2, gdouble g2, gdouble b2);
TboAction * tbo_action_frame_move_new (Frame *frame, int x1, int y1, int x2, int y2);
TboAction * tbo_action_frame_transform_new (Frame *frame,
                                            int x1,
                                            int y1,
                                            int width1,
                                            int height1,
                                            int x2,
                                            int y2,
                                            int width2,
                                            int height2);
TboAction * tbo_action_object_flags_new (TboObjectBase *object,
                                         gboolean flipv1,
                                         gboolean fliph1,
                                         gboolean flipv2,
                                         gboolean fliph2);
TboAction * tbo_action_object_order_new (Frame *frame,
                                         TboObjectBase *object,
                                         int index1,
                                         int index2);
TboAction * tbo_action_object_move_new (TboObjectBase *object, int x1, int y1, int x2, int y2);
TboAction * tbo_action_object_transform_new (TboObjectBase *object,
                                             int x1,
                                             int y1,
                                             int width1,
                                             int height1,
                                             gdouble angle1,
                                             int x2,
                                             int y2,
                                             int width2,
                                             int height2,
                                             gdouble angle2);
TboAction * tbo_action_text_state_new (TboObjectText *object,
                                       const gchar *text1,
                                       const gchar *font1,
                                       const GdkRGBA *color1,
                                       const gchar *text2,
                                       const gchar *font2,
                                       const GdkRGBA *color2);

#endif
