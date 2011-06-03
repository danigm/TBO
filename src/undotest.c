#include <stdio.h>
#include "tbo-undo.h"

// Defining a custom TboAction with custom data
typedef struct _TboActionString TboActionString;

struct _TboActionString {
    void (*action_do) (TboAction *action);
    void (*action_undo) (TboAction *action);
    char *data;
};

void
testdo (TboAction *act) {
    TboActionString *action = (TboActionString*)act;
    printf (" + doing %s\n", action->data);
}

void
testundo (TboAction *act) {
    TboActionString *action = (TboActionString*)act;
    printf (" - UNdoing %s\n", action->data);
}

TboAction *
tbo_action_string_new (char *str) {
    TboAction *act = tbo_action_new (TboActionString);
    TboActionString *action = (TboActionString*)act;
    action->data = str;
    tbo_action_set (act, testdo, testundo);
    return act;
}

int
main (int argc, char **argv)
{
    TboUndoStack *stack;
    TboAction *action;

    printf ("Testing TBO undo\n");

    stack = tbo_undo_stack_new ();

    action = tbo_action_string_new ("Test action1");
    tbo_undo_stack_insert (stack, action);
    action = tbo_action_string_new ("Test action2");
    tbo_undo_stack_insert (stack, action);
    action = tbo_action_string_new ("Test action3");
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
    action = tbo_action_string_new ("Test action4");
    tbo_undo_stack_insert (stack, action);
    tbo_undo_stack_redo (stack);
    tbo_undo_stack_undo (stack);
    tbo_undo_stack_undo (stack);
    tbo_undo_stack_redo (stack);

    printf ("All OK\n");
    return 0;
}
