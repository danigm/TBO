#include <stdio.h>
#include "tbo-undo.h"

void
testdo (TboAction *action) {
    printf (" + doing %s\n", action->data);
}

void
testundo (TboAction *action) {
    printf (" - UNdoing %s\n", action->data);
}

int
main (int argc, char **argv)
{
    TboUndoStack *stack;
    TboAction *action;

    printf ("Testing TBO undo\n");

    stack = tbo_undo_stack_new ();

    action = tbo_action_new ("Test action1", testdo, testundo);
    tbo_undo_stack_insert (stack, action);
    action = tbo_action_new ("Test action2", testdo, testundo);
    tbo_undo_stack_insert (stack, action);
    action = tbo_action_new ("Test action3", testdo, testundo);
    tbo_undo_stack_insert (stack, action);

    tbo_undo_stack_undo (stack);
    tbo_undo_stack_undo (stack);
    tbo_undo_stack_undo (stack);

    printf ("\nUndoing nothing\n");
    printf ("problem?\n");
    tbo_undo_stack_undo (stack);
    printf ("problem?\n");
    tbo_undo_stack_undo (stack);

    printf ("\nNow redoing\n");
    // redoing
    tbo_undo_stack_redo (stack);
    tbo_undo_stack_redo (stack);
    tbo_undo_stack_redo (stack);

    printf ("\nRedoing nothing\n");
    printf ("problem?\n");
    tbo_undo_stack_redo (stack);
    printf ("problem?\n");
    tbo_undo_stack_redo (stack);

    printf ("\nNow undo and redo\n");
    tbo_undo_stack_undo (stack);
    action = tbo_action_new ("Test action4", testdo, testundo);
    tbo_undo_stack_insert (stack, action);
    tbo_undo_stack_redo (stack);
    tbo_undo_stack_undo (stack);
    tbo_undo_stack_undo (stack);
    tbo_undo_stack_redo (stack);

    printf ("All OK\n");
    return 0;
}
