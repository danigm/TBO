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

typedef struct _TboAction TboAction;
typedef struct _TboUndoStack TboUndoStack;

struct _TboAction {
    gpointer data;
    void (*action_do) (TboAction *action);
    void (*action_undo) (TboAction *action);
};

TboAction * tbo_action_new (gpointer data, gpointer action_do, gpointer action_undo);
void tbo_action_del (TboAction *action);

struct _TboUndoStack {
    GList *first;
    GList *list;
    gboolean last_flag;
    gboolean first_flag;
};

TboUndoStack * tbo_undo_stack_new ();
void tbo_undo_stack_del ();
void tbo_undo_stack_insert (TboUndoStack *stack, TboAction *action);
void tbo_undo_stack_undo (TboUndoStack *stack);
void tbo_undo_stack_redo (TboUndoStack *stack);

#endif
