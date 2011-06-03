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
#include "tbo-undo.h"

void
tbo_action_set (TboAction *action,
                gpointer action_do,
                gpointer action_undo)
{
    action->action_do = action_do;
    action->action_undo = action_undo;
}

void
tbo_action_del (TboAction *action)
{
    free (action);
}

void
tbo_action_del_data (TboAction *action, gpointer user_data)
{
    free (action);
}

TboUndoStack *
tbo_undo_stack_new ()
{
    TboUndoStack *stack = malloc (sizeof (TboUndoStack));
    stack->first = NULL;
    stack->list = NULL;
    stack->last_flag = FALSE;
    stack->first_flag = FALSE;
    return stack;
}

void
tbo_undo_stack_insert (TboUndoStack *stack, TboAction *action)
{
    // Removing each element before the actual one
    if (stack->first) {
        while (stack->first != stack->list) {
            tbo_action_del ((TboAction*)((stack->first)->data));
            stack->first = g_list_remove_link (stack->first, stack->first);
        }
    }

    stack->last_flag = FALSE;
    stack->list = g_list_prepend (stack->list, (gpointer)action);
    stack->first = stack->list;
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
}

void
tbo_undo_stack_del (TboUndoStack *stack)
{
    g_list_foreach (stack->first, (GFunc)tbo_action_del_data, NULL);
    free (stack);
}
