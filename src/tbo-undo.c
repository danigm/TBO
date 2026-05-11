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

#include <stdlib.h>
#include <string.h>
#include "comic.h"
#include "page.h"
#include "frame.h"
#include "tbo-object-base.h"
#include "tbo-object-text.h"
#include "tbo-undo.h"

typedef struct
{
    TboAction base;
    Page *page;
    Frame *frame;
    gint index;
} TboActionFrameAdd;

typedef struct
{
    TboAction base;
    Comic *comic;
    Page *page;
    gint index;
} TboActionPageChange;

typedef struct
{
    TboAction base;
    Comic *comic;
    Page *page;
    gint index1;
    gint index2;
} TboActionPageOrder;

typedef struct
{
    TboAction base;
    Frame *frame;
    TboObjectBase *obj;
    gint index;
} TboActionObjectAdd;

typedef struct
{
    TboAction base;
    Page *page;
    Frame *frame;
    gint index;
} TboActionFrameChange;

typedef struct
{
    TboAction base;
    Frame *frame;
    TboObjectBase *obj;
    gint index;
} TboActionObjectChange;

typedef struct
{
    TboAction base;
    Frame *frame;
    gint x1, y1, width1, height1;
    gboolean border1;
    gdouble r1, g1, b1;
    gint x2, y2, width2, height2;
    gboolean border2;
    gdouble r2, g2, b2;
} TboActionFrameState;

typedef struct
{
    TboAction base;
    Frame *frame;
    gint x1;
    gint y1;
    gint x2;
    gint y2;
} TboActionFrameMove;

typedef struct
{
    TboAction base;
    Frame *frame;
    gint x1;
    gint y1;
    gint width1;
    gint height1;
    gint x2;
    gint y2;
    gint width2;
    gint height2;
} TboActionFrameTransform;

typedef struct
{
    TboAction base;
    TboObjectBase *obj;
    gboolean flipv1;
    gboolean fliph1;
    gboolean flipv2;
    gboolean fliph2;
} TboActionObjectFlags;

typedef struct
{
    TboAction base;
    Frame *frame;
    TboObjectBase *obj;
    gint index1;
    gint index2;
} TboActionObjectOrder;

typedef struct
{
    TboAction base;
    TboObjectBase *obj;
    gint x1;
    gint y1;
    gint x2;
    gint y2;
} TboActionObjMove;

typedef struct
{
    TboAction base;
    TboObjectBase *obj;
    gint x1;
    gint y1;
    gint width1;
    gint height1;
    gdouble angle1;
    gint x2;
    gint y2;
    gint width2;
    gint height2;
    gdouble angle2;
} TboActionObjTransform;

typedef struct
{
    TboAction base;
    TboObjectText *obj;
    gchar *text1;
    gchar *font1;
    GdkRGBA color1;
    gchar *text2;
    gchar *font2;
    GdkRGBA color2;
} TboActionTextState;

static void
free_action_link (GList *link)
{
    if (link == NULL)
        return;

    tbo_action_del ((TboAction *) link->data);
    g_list_free_1 (link);
}

static gboolean
page_has_frame (Page *page, Frame *frame)
{
    return page != NULL && frame != NULL && g_list_find (tbo_page_get_frames (page), frame) != NULL;
}

static gboolean
comic_has_page (Comic *comic, Page *page)
{
    return comic != NULL && page != NULL && g_list_find (tbo_comic_get_pages (comic), page) != NULL;
}

static void
frame_add_do (TboAction *action)
{
    TboActionFrameAdd *frame_action = (TboActionFrameAdd *) action;

    if (frame_action->page == NULL || frame_action->frame == NULL || page_has_frame (frame_action->page, frame_action->frame))
        return;

    g_object_ref (frame_action->frame);
    tbo_page_insert_frame (frame_action->page, frame_action->frame, frame_action->index);
}

static void
frame_add_undo (TboAction *action)
{
    TboActionFrameAdd *frame_action = (TboActionFrameAdd *) action;

    if (frame_action->page == NULL || frame_action->frame == NULL || !page_has_frame (frame_action->page, frame_action->frame))
        return;

    tbo_page_del_frame (frame_action->page, frame_action->frame);
}

static void
frame_add_free (TboAction *action)
{
    TboActionFrameAdd *frame_action = (TboActionFrameAdd *) action;

    if (frame_action->page != NULL)
        g_object_remove_weak_pointer (G_OBJECT (frame_action->page), (gpointer *) &frame_action->page);
    if (frame_action->frame != NULL)
        g_object_unref (frame_action->frame);
}

static void
object_add_do (TboAction *action)
{
    TboActionObjectAdd *obj_action = (TboActionObjectAdd *) action;

    if (obj_action->frame == NULL || obj_action->obj == NULL || tbo_frame_has_obj (obj_action->frame, obj_action->obj))
        return;

    g_object_ref (obj_action->obj);
    tbo_frame_insert_obj (obj_action->frame, obj_action->obj, obj_action->index);
}

static void
object_add_undo (TboAction *action)
{
    TboActionObjectAdd *obj_action = (TboActionObjectAdd *) action;

    if (obj_action->frame == NULL || obj_action->obj == NULL || !tbo_frame_has_obj (obj_action->frame, obj_action->obj))
        return;

    tbo_frame_del_obj (obj_action->frame, obj_action->obj);
}

static void
object_add_free (TboAction *action)
{
    TboActionObjectAdd *obj_action = (TboActionObjectAdd *) action;

    if (obj_action->frame != NULL)
        g_object_remove_weak_pointer (G_OBJECT (obj_action->frame), (gpointer *) &obj_action->frame);
    if (obj_action->obj != NULL)
        g_object_unref (obj_action->obj);
}

static void
page_add_do (TboAction *action)
{
    TboActionPageChange *page_action = (TboActionPageChange *) action;

    if (page_action->comic == NULL || page_action->page == NULL || comic_has_page (page_action->comic, page_action->page))
        return;

    g_object_ref (page_action->page);
    tbo_comic_insert_page (page_action->comic, page_action->page, page_action->index);
    tbo_comic_set_current_page (page_action->comic, page_action->page);
}

static void
page_add_undo (TboAction *action)
{
    TboActionPageChange *page_action = (TboActionPageChange *) action;
    gint index;

    if (page_action->comic == NULL || page_action->page == NULL || !comic_has_page (page_action->comic, page_action->page))
        return;

    index = tbo_comic_page_nth (page_action->comic, page_action->page);
    if (index >= 0)
        tbo_comic_del_page (page_action->comic, index);
}

static void
page_remove_do (TboAction *action)
{
    page_add_undo (action);
}

static void
page_remove_undo (TboAction *action)
{
    page_add_do (action);
}

static void
page_change_free (TboAction *action)
{
    TboActionPageChange *page_action = (TboActionPageChange *) action;

    if (page_action->comic != NULL)
        g_object_remove_weak_pointer (G_OBJECT (page_action->comic), (gpointer *) &page_action->comic);
    if (page_action->page != NULL)
        g_object_unref (page_action->page);
}

static void
page_order_do (TboAction *action)
{
    TboActionPageOrder *page_action = (TboActionPageOrder *) action;

    if (page_action->comic == NULL || page_action->page == NULL || !comic_has_page (page_action->comic, page_action->page))
        return;

    tbo_comic_reorder_page (page_action->comic, page_action->page, page_action->index2);
    tbo_comic_set_current_page (page_action->comic, page_action->page);
}

static void
page_order_undo (TboAction *action)
{
    TboActionPageOrder *page_action = (TboActionPageOrder *) action;

    if (page_action->comic == NULL || page_action->page == NULL || !comic_has_page (page_action->comic, page_action->page))
        return;

    tbo_comic_reorder_page (page_action->comic, page_action->page, page_action->index1);
    tbo_comic_set_current_page (page_action->comic, page_action->page);
}

static void
page_order_free (TboAction *action)
{
    TboActionPageOrder *page_action = (TboActionPageOrder *) action;

    if (page_action->comic != NULL)
        g_object_remove_weak_pointer (G_OBJECT (page_action->comic), (gpointer *) &page_action->comic);
    if (page_action->page != NULL)
        g_object_unref (page_action->page);
}

static void
frame_remove_do (TboAction *action)
{
    TboActionFrameChange *frame_action = (TboActionFrameChange *) action;

    if (frame_action->page == NULL || frame_action->frame == NULL || !page_has_frame (frame_action->page, frame_action->frame))
        return;

    tbo_page_del_frame (frame_action->page, frame_action->frame);
}

static void
frame_remove_undo (TboAction *action)
{
    TboActionFrameChange *frame_action = (TboActionFrameChange *) action;

    if (frame_action->page == NULL || frame_action->frame == NULL || page_has_frame (frame_action->page, frame_action->frame))
        return;

    g_object_ref (frame_action->frame);
    tbo_page_insert_frame (frame_action->page, frame_action->frame, frame_action->index);
}

static void
frame_change_free (TboAction *action)
{
    TboActionFrameChange *frame_action = (TboActionFrameChange *) action;

    if (frame_action->page != NULL)
        g_object_remove_weak_pointer (G_OBJECT (frame_action->page), (gpointer *) &frame_action->page);
    if (frame_action->frame != NULL)
        g_object_unref (frame_action->frame);
}

static void
object_remove_do (TboAction *action)
{
    TboActionObjectChange *obj_action = (TboActionObjectChange *) action;

    if (obj_action->frame == NULL || obj_action->obj == NULL || !tbo_frame_has_obj (obj_action->frame, obj_action->obj))
        return;

    tbo_frame_del_obj (obj_action->frame, obj_action->obj);
}

static void
object_remove_undo (TboAction *action)
{
    TboActionObjectChange *obj_action = (TboActionObjectChange *) action;

    if (obj_action->frame == NULL || obj_action->obj == NULL || tbo_frame_has_obj (obj_action->frame, obj_action->obj))
        return;

    g_object_ref (obj_action->obj);
    tbo_frame_insert_obj (obj_action->frame, obj_action->obj, obj_action->index);
}

static void
object_change_free (TboAction *action)
{
    TboActionObjectChange *obj_action = (TboActionObjectChange *) action;

    if (obj_action->frame != NULL)
        g_object_remove_weak_pointer (G_OBJECT (obj_action->frame), (gpointer *) &obj_action->frame);
    if (obj_action->obj != NULL)
        g_object_unref (obj_action->obj);
}

static void
apply_frame_state (TboActionFrameState *action,
                   gint x,
                   gint y,
                   gint width,
                   gint height,
                   gboolean border,
                   gdouble r,
                   gdouble g,
                   gdouble b)
{
    if (action->frame == NULL)
        return;

    tbo_frame_set_bounds (action->frame, x, y, width, height);
    tbo_frame_set_border (action->frame, border);
    tbo_frame_set_color_rgb (action->frame, r, g, b);
}

static void
apply_frame_position (Frame *frame, gint x, gint y)
{
    if (frame != NULL)
        tbo_frame_set_position (frame, x, y);
}

static void
apply_frame_transform (TboActionFrameTransform *action,
                       gint x,
                       gint y,
                       gint width,
                       gint height)
{
    if (action->frame != NULL)
        tbo_frame_set_bounds (action->frame, x, y, width, height);
}

static void
apply_object_position (TboObjectBase *obj, gint x, gint y)
{
    if (obj != NULL)
    {
        obj->x = x;
        obj->y = y;
    }
}

static void
apply_object_transform (TboActionObjTransform *action,
                        gint x,
                        gint y,
                        gint width,
                        gint height,
                        gdouble angle)
{
    if (action->obj == NULL)
        return;

    action->obj->x = x;
    action->obj->y = y;
    action->obj->width = width;
    action->obj->height = height;
    action->obj->angle = angle;
}

static void
frame_state_do (TboAction *action)
{
    TboActionFrameState *frame_action = (TboActionFrameState *) action;

    apply_frame_state (frame_action,
                       frame_action->x2, frame_action->y2,
                       frame_action->width2, frame_action->height2,
                       frame_action->border2,
                       frame_action->r2, frame_action->g2, frame_action->b2);
}

static void
frame_move_do (TboAction *action)
{
    TboActionFrameMove *frame_action = (TboActionFrameMove *) action;

    apply_frame_position (frame_action->frame, frame_action->x2, frame_action->y2);
}

static void
frame_move_undo (TboAction *action)
{
    TboActionFrameMove *frame_action = (TboActionFrameMove *) action;

    apply_frame_position (frame_action->frame, frame_action->x1, frame_action->y1);
}

static void
frame_move_free (TboAction *action)
{
    TboActionFrameMove *frame_action = (TboActionFrameMove *) action;

    if (frame_action->frame != NULL)
        g_object_remove_weak_pointer (G_OBJECT (frame_action->frame), (gpointer *) &frame_action->frame);
}

static void
frame_transform_do (TboAction *action)
{
    TboActionFrameTransform *frame_action = (TboActionFrameTransform *) action;

    apply_frame_transform (frame_action,
                           frame_action->x2,
                           frame_action->y2,
                           frame_action->width2,
                           frame_action->height2);
}

static void
frame_state_undo (TboAction *action)
{
    TboActionFrameState *frame_action = (TboActionFrameState *) action;

    apply_frame_state (frame_action,
                       frame_action->x1, frame_action->y1,
                       frame_action->width1, frame_action->height1,
                       frame_action->border1,
                       frame_action->r1, frame_action->g1, frame_action->b1);
}

static void
frame_transform_undo (TboAction *action)
{
    TboActionFrameTransform *frame_action = (TboActionFrameTransform *) action;

    apply_frame_transform (frame_action,
                           frame_action->x1,
                           frame_action->y1,
                           frame_action->width1,
                           frame_action->height1);
}

static void
frame_state_free (TboAction *action)
{
    TboActionFrameState *frame_action = (TboActionFrameState *) action;

    if (frame_action->frame != NULL)
        g_object_remove_weak_pointer (G_OBJECT (frame_action->frame), (gpointer *) &frame_action->frame);
}

static void
frame_transform_free (TboAction *action)
{
    TboActionFrameTransform *frame_action = (TboActionFrameTransform *) action;

    if (frame_action->frame != NULL)
        g_object_remove_weak_pointer (G_OBJECT (frame_action->frame), (gpointer *) &frame_action->frame);
}

static void
object_flags_do (TboAction *action)
{
    TboActionObjectFlags *obj_action = (TboActionObjectFlags *) action;

    if (obj_action->obj == NULL)
        return;

    obj_action->obj->flipv = obj_action->flipv2;
    obj_action->obj->fliph = obj_action->fliph2;
}

static void
object_move_do (TboAction *action)
{
    TboActionObjMove *obj_action = (TboActionObjMove *) action;

    apply_object_position (obj_action->obj, obj_action->x2, obj_action->y2);
}

static void
object_move_undo (TboAction *action)
{
    TboActionObjMove *obj_action = (TboActionObjMove *) action;

    apply_object_position (obj_action->obj, obj_action->x1, obj_action->y1);
}

static void
object_move_free (TboAction *action)
{
    TboActionObjMove *obj_action = (TboActionObjMove *) action;

    if (obj_action->obj != NULL)
        g_object_remove_weak_pointer (G_OBJECT (obj_action->obj), (gpointer *) &obj_action->obj);
}

static void
object_transform_do (TboAction *action)
{
    TboActionObjTransform *obj_action = (TboActionObjTransform *) action;

    apply_object_transform (obj_action,
                            obj_action->x2,
                            obj_action->y2,
                            obj_action->width2,
                            obj_action->height2,
                            obj_action->angle2);
}

static void
object_flags_undo (TboAction *action)
{
    TboActionObjectFlags *obj_action = (TboActionObjectFlags *) action;

    if (obj_action->obj == NULL)
        return;

    obj_action->obj->flipv = obj_action->flipv1;
    obj_action->obj->fliph = obj_action->fliph1;
}

static void
object_transform_undo (TboAction *action)
{
    TboActionObjTransform *obj_action = (TboActionObjTransform *) action;

    apply_object_transform (obj_action,
                            obj_action->x1,
                            obj_action->y1,
                            obj_action->width1,
                            obj_action->height1,
                            obj_action->angle1);
}

static void
object_flags_free (TboAction *action)
{
    TboActionObjectFlags *obj_action = (TboActionObjectFlags *) action;

    if (obj_action->obj != NULL)
        g_object_remove_weak_pointer (G_OBJECT (obj_action->obj), (gpointer *) &obj_action->obj);
}

static void
object_transform_free (TboAction *action)
{
    TboActionObjTransform *obj_action = (TboActionObjTransform *) action;

    if (obj_action->obj != NULL)
        g_object_remove_weak_pointer (G_OBJECT (obj_action->obj), (gpointer *) &obj_action->obj);
}

static void
object_order_do (TboAction *action)
{
    TboActionObjectOrder *obj_action = (TboActionObjectOrder *) action;

    if (obj_action->frame == NULL || obj_action->obj == NULL || !tbo_frame_has_obj (obj_action->frame, obj_action->obj))
        return;

    tbo_frame_reorder_obj (obj_action->frame, obj_action->obj, obj_action->index2);
}

static void
object_order_undo (TboAction *action)
{
    TboActionObjectOrder *obj_action = (TboActionObjectOrder *) action;

    if (obj_action->frame == NULL || obj_action->obj == NULL || !tbo_frame_has_obj (obj_action->frame, obj_action->obj))
        return;

    tbo_frame_reorder_obj (obj_action->frame, obj_action->obj, obj_action->index1);
}

static void
object_order_free (TboAction *action)
{
    TboActionObjectOrder *obj_action = (TboActionObjectOrder *) action;

    if (obj_action->frame != NULL)
        g_object_remove_weak_pointer (G_OBJECT (obj_action->frame), (gpointer *) &obj_action->frame);
    if (obj_action->obj != NULL)
        g_object_remove_weak_pointer (G_OBJECT (obj_action->obj), (gpointer *) &obj_action->obj);
}

static void
apply_text_state (TboActionTextState *action,
                  const gchar *text,
                  const gchar *font,
                  const GdkRGBA *color)
{
    if (action->obj == NULL)
        return;

    tbo_object_text_set_text (action->obj, text);
    tbo_object_text_change_font (action->obj, (gchar *) font);
    tbo_object_text_change_color (action->obj, (GdkRGBA *) color);
}

static void
text_state_do (TboAction *action)
{
    TboActionTextState *text_action = (TboActionTextState *) action;

    apply_text_state (text_action, text_action->text2, text_action->font2, &text_action->color2);
}

static void
text_state_undo (TboAction *action)
{
    TboActionTextState *text_action = (TboActionTextState *) action;

    apply_text_state (text_action, text_action->text1, text_action->font1, &text_action->color1);
}

static void
text_state_free (TboAction *action)
{
    TboActionTextState *text_action = (TboActionTextState *) action;

    if (text_action->obj != NULL)
        g_object_remove_weak_pointer (G_OBJECT (text_action->obj), (gpointer *) &text_action->obj);
    g_free (text_action->text1);
    g_free (text_action->font1);
    g_free (text_action->text2);
    g_free (text_action->font2);
}

void
tbo_action_del (TboAction *action)
{
    if (action->action_free != NULL)
        action->action_free (action);

    free (action);
}

TboUndoStack *
tbo_undo_stack_new (void)
{
    TboUndoStack *stack = malloc (sizeof (TboUndoStack));
    stack->first = NULL;
    stack->list = NULL;
    stack->last_flag = TRUE;
    stack->current_state_id = 0;
    stack->next_state_id = 1;
    return stack;
}

void
tbo_undo_stack_insert (TboUndoStack *stack, TboAction *action)
{
    action->state_before = stack->current_state_id;
    action->state_after = stack->next_state_id++;

    // Removing each element before the actual one
    if (stack->first)
    {
        if (stack->last_flag)
            tbo_undo_stack_clear (stack);

        while (stack->first != NULL && stack->first != stack->list)
        {
            GList *link = stack->first;

            stack->first = stack->first->next;
            if (stack->first != NULL)
                stack->first->prev = NULL;

            free_action_link (link);
        }
    }

    stack->last_flag = FALSE;
    stack->list = g_list_prepend (stack->list, (gpointer)action);
    stack->first = stack->list;
    stack->current_state_id = action->state_after;
}

void
tbo_undo_stack_clear (TboUndoStack *stack)
{
    GList *link;

    if (stack == NULL)
        return;

    link = stack->first;
    while (link != NULL)
    {
        GList *next = link->next;

        free_action_link (link);
        link = next;
    }

    stack->first = NULL;
    stack->list = NULL;
    stack->last_flag = TRUE;
    stack->next_state_id = stack->current_state_id + 1;
}

void
tbo_undo_stack_undo (TboUndoStack *stack)
{
    if (!stack->list)
        return;

    if (stack->last_flag)
        return;

    // undo
    TboAction *action = NULL;
    action = (stack->list)->data;
    tbo_action_undo (action);
    stack->current_state_id = action->state_before;

    if (stack->list->next)
        stack->list = (stack->list)->next;
    else
        stack->last_flag = TRUE;
}

void
tbo_undo_stack_redo (TboUndoStack *stack)
{
    if (!stack->list)
        return;

    if (stack->last_flag)
        stack->last_flag = FALSE;
    else if (stack->list != stack->first)
        stack->list = (stack->list)->prev;
    else
        return;


    // redo
    TboAction *action = NULL;
    action = (stack->list)->data;
    tbo_action_do (action);
    stack->current_state_id = action->state_after;
}

void
tbo_undo_stack_del (TboUndoStack *stack)
{
    tbo_undo_stack_clear (stack);
    free (stack);
}


gboolean
tbo_undo_active_undo (TboUndoStack *stack)
{
    return !stack->last_flag;
}

gboolean
tbo_undo_active_redo (TboUndoStack *stack)
{
    return stack->first != stack->list;
}

TboAction *
tbo_action_frame_add_new (Page *page, Frame *frame)
{
    TboActionFrameAdd *action = (TboActionFrameAdd *) tbo_action_new (TboActionFrameAdd);

    action->page = page;
    action->frame = g_object_ref (frame);
    action->index = tbo_page_frame_nth (page, frame);
    action->base.action_do = frame_add_do;
    action->base.action_undo = frame_add_undo;
    action->base.action_free = frame_add_free;

    if (action->page != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->page), (gpointer *) &action->page);

    return (TboAction *) action;
}

TboAction *
tbo_action_object_add_new (Frame *frame, TboObjectBase *object)
{
    TboActionObjectAdd *action = (TboActionObjectAdd *) tbo_action_new (TboActionObjectAdd);

    action->frame = frame;
    action->obj = g_object_ref (object);
    action->index = tbo_frame_object_nth (frame, object);
    action->base.action_do = object_add_do;
    action->base.action_undo = object_add_undo;
    action->base.action_free = object_add_free;

    if (action->frame != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->frame), (gpointer *) &action->frame);

    return (TboAction *) action;
}

TboAction *
tbo_action_page_add_new (Comic *comic, Page *page, int index)
{
    TboActionPageChange *action = (TboActionPageChange *) tbo_action_new (TboActionPageChange);

    action->comic = comic;
    action->page = g_object_ref (page);
    action->index = index;
    action->base.action_do = page_add_do;
    action->base.action_undo = page_add_undo;
    action->base.action_free = page_change_free;

    if (action->comic != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->comic), (gpointer *) &action->comic);

    return (TboAction *) action;
}

TboAction *
tbo_action_page_remove_new (Comic *comic, Page *page, int index)
{
    TboActionPageChange *action = (TboActionPageChange *) tbo_action_new (TboActionPageChange);

    action->comic = comic;
    action->page = g_object_ref (page);
    action->index = index;
    action->base.action_do = page_remove_do;
    action->base.action_undo = page_remove_undo;
    action->base.action_free = page_change_free;

    if (action->comic != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->comic), (gpointer *) &action->comic);

    return (TboAction *) action;
}

TboAction *
tbo_action_page_reorder_new (Comic *comic, Page *page, int index1, int index2)
{
    TboActionPageOrder *action = (TboActionPageOrder *) tbo_action_new (TboActionPageOrder);

    action->comic = comic;
    action->page = g_object_ref (page);
    action->index1 = index1;
    action->index2 = index2;
    action->base.action_do = page_order_do;
    action->base.action_undo = page_order_undo;
    action->base.action_free = page_order_free;

    if (action->comic != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->comic), (gpointer *) &action->comic);

    return (TboAction *) action;
}

TboAction *
tbo_action_frame_remove_new (Page *page, Frame *frame, int index)
{
    TboActionFrameChange *action = (TboActionFrameChange *) tbo_action_new (TboActionFrameChange);

    action->page = page;
    action->frame = g_object_ref (frame);
    action->index = index;
    action->base.action_do = frame_remove_do;
    action->base.action_undo = frame_remove_undo;
    action->base.action_free = frame_change_free;

    if (action->page != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->page), (gpointer *) &action->page);

    return (TboAction *) action;
}

TboAction *
tbo_action_object_remove_new (Frame *frame, TboObjectBase *object, int index)
{
    TboActionObjectChange *action = (TboActionObjectChange *) tbo_action_new (TboActionObjectChange);

    action->frame = frame;
    action->obj = g_object_ref (object);
    action->index = index;
    action->base.action_do = object_remove_do;
    action->base.action_undo = object_remove_undo;
    action->base.action_free = object_change_free;

    if (action->frame != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->frame), (gpointer *) &action->frame);

    return (TboAction *) action;
}

TboAction *
tbo_action_frame_state_new (Frame *frame,
                            int x1, int y1, int width1, int height1,
                            gboolean border1, gdouble r1, gdouble g1, gdouble b1,
                            int x2, int y2, int width2, int height2,
                            gboolean border2, gdouble r2, gdouble g2, gdouble b2)
{
    TboActionFrameState *action = (TboActionFrameState *) tbo_action_new (TboActionFrameState);

    action->frame = frame;
    action->x1 = x1; action->y1 = y1; action->width1 = width1; action->height1 = height1;
    action->border1 = border1; action->r1 = r1; action->g1 = g1; action->b1 = b1;
    action->x2 = x2; action->y2 = y2; action->width2 = width2; action->height2 = height2;
    action->border2 = border2; action->r2 = r2; action->g2 = g2; action->b2 = b2;
    action->base.action_do = frame_state_do;
    action->base.action_undo = frame_state_undo;
    action->base.action_free = frame_state_free;

    if (action->frame != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->frame), (gpointer *) &action->frame);

    return (TboAction *) action;
}

TboAction *
tbo_action_frame_move_new (Frame *frame, int x1, int y1, int x2, int y2)
{
    TboActionFrameMove *action = (TboActionFrameMove *) tbo_action_new (TboActionFrameMove);

    action->frame = frame;
    action->x1 = x1;
    action->y1 = y1;
    action->x2 = x2;
    action->y2 = y2;
    action->base.action_do = frame_move_do;
    action->base.action_undo = frame_move_undo;
    action->base.action_free = frame_move_free;

    if (action->frame != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->frame), (gpointer *) &action->frame);

    return (TboAction *) action;
}

TboAction *
tbo_action_frame_transform_new (Frame *frame,
                                int x1,
                                int y1,
                                int width1,
                                int height1,
                                int x2,
                                int y2,
                                int width2,
                                int height2)
{
    TboActionFrameTransform *action = (TboActionFrameTransform *) tbo_action_new (TboActionFrameTransform);

    action->frame = frame;
    action->x1 = x1;
    action->y1 = y1;
    action->width1 = width1;
    action->height1 = height1;
    action->x2 = x2;
    action->y2 = y2;
    action->width2 = width2;
    action->height2 = height2;
    action->base.action_do = frame_transform_do;
    action->base.action_undo = frame_transform_undo;
    action->base.action_free = frame_transform_free;

    if (action->frame != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->frame), (gpointer *) &action->frame);

    return (TboAction *) action;
}

TboAction *
tbo_action_object_flags_new (TboObjectBase *object,
                             gboolean flipv1,
                             gboolean fliph1,
                             gboolean flipv2,
                             gboolean fliph2)
{
    TboActionObjectFlags *action = (TboActionObjectFlags *) tbo_action_new (TboActionObjectFlags);

    action->obj = object;
    action->flipv1 = flipv1;
    action->fliph1 = fliph1;
    action->flipv2 = flipv2;
    action->fliph2 = fliph2;
    action->base.action_do = object_flags_do;
    action->base.action_undo = object_flags_undo;
    action->base.action_free = object_flags_free;

    if (action->obj != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->obj), (gpointer *) &action->obj);

    return (TboAction *) action;
}

TboAction *
tbo_action_object_order_new (Frame *frame,
                             TboObjectBase *object,
                             int index1,
                             int index2)
{
    TboActionObjectOrder *action = (TboActionObjectOrder *) tbo_action_new (TboActionObjectOrder);

    action->frame = frame;
    action->obj = object;
    action->index1 = index1;
    action->index2 = index2;
    action->base.action_do = object_order_do;
    action->base.action_undo = object_order_undo;
    action->base.action_free = object_order_free;

    if (action->frame != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->frame), (gpointer *) &action->frame);
    if (action->obj != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->obj), (gpointer *) &action->obj);

    return (TboAction *) action;
}

TboAction *
tbo_action_object_move_new (TboObjectBase *object, int x1, int y1, int x2, int y2)
{
    TboActionObjMove *action = (TboActionObjMove *) tbo_action_new (TboActionObjMove);

    action->obj = object;
    action->x1 = x1;
    action->y1 = y1;
    action->x2 = x2;
    action->y2 = y2;
    action->base.action_do = object_move_do;
    action->base.action_undo = object_move_undo;
    action->base.action_free = object_move_free;

    if (action->obj != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->obj), (gpointer *) &action->obj);

    return (TboAction *) action;
}

TboAction *
tbo_action_object_transform_new (TboObjectBase *object,
                                 int x1,
                                 int y1,
                                 int width1,
                                 int height1,
                                 gdouble angle1,
                                 int x2,
                                 int y2,
                                 int width2,
                                 int height2,
                                 gdouble angle2)
{
    TboActionObjTransform *action = (TboActionObjTransform *) tbo_action_new (TboActionObjTransform);

    action->obj = object;
    action->x1 = x1;
    action->y1 = y1;
    action->width1 = width1;
    action->height1 = height1;
    action->angle1 = angle1;
    action->x2 = x2;
    action->y2 = y2;
    action->width2 = width2;
    action->height2 = height2;
    action->angle2 = angle2;
    action->base.action_do = object_transform_do;
    action->base.action_undo = object_transform_undo;
    action->base.action_free = object_transform_free;

    if (action->obj != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->obj), (gpointer *) &action->obj);

    return (TboAction *) action;
}

TboAction *
tbo_action_text_state_new (TboObjectText *object,
                           const gchar *text1,
                           const gchar *font1,
                           const GdkRGBA *color1,
                           const gchar *text2,
                           const gchar *font2,
                           const GdkRGBA *color2)
{
    TboActionTextState *action = (TboActionTextState *) tbo_action_new (TboActionTextState);

    action->obj = object;
    action->text1 = g_strdup (text1);
    action->font1 = g_strdup (font1);
    action->color1 = *color1;
    action->text2 = g_strdup (text2);
    action->font2 = g_strdup (font2);
    action->color2 = *color2;
    action->base.action_do = text_state_do;
    action->base.action_undo = text_state_undo;
    action->base.action_free = text_state_free;

    if (action->obj != NULL)
        g_object_add_weak_pointer (G_OBJECT (action->obj), (gpointer *) &action->obj);

    return (TboAction *) action;
}
