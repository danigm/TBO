#ifndef __TBO_LIST_UTILS_H__
#define __TBO_LIST_UTILS_H__

#include <glib.h>

static inline GList *
tbo_list_utils_link (GList *list, gpointer item)
{
    return g_list_find (list, item);
}

static inline gboolean
tbo_list_utils_contains (GList *list, gpointer item)
{
    return tbo_list_utils_link (list, item) != NULL;
}

static inline void
tbo_list_utils_insert (GList **list, gpointer item, gint nth)
{
    if (nth < 0)
        *list = g_list_append (*list, item);
    else
        *list = g_list_insert (*list, item, nth);
}

static inline gboolean
tbo_list_utils_remove (GList **list, gpointer item)
{
    GList *link = tbo_list_utils_link (*list, item);

    if (link == NULL)
        return FALSE;

    *list = g_list_delete_link (*list, link);
    return TRUE;
}

static inline void
tbo_current_list_insert (GList **list, gpointer *current, gpointer item, gint nth)
{
    tbo_list_utils_insert (list, item, nth);
    if (current != NULL && *current == NULL)
        *current = item;
}

static inline gboolean
tbo_current_list_remove (GList **list, gpointer *current, gpointer item)
{
    GList *link = tbo_list_utils_link (*list, item);
    GList *fallback;

    if (link == NULL)
        return FALSE;

    fallback = link->next != NULL ? link->next : link->prev;
    *list = g_list_delete_link (*list, link);

    if (current != NULL && *current == item)
        *current = fallback != NULL ? fallback->data : NULL;

    return TRUE;
}

static inline gint
tbo_current_list_index (GList *list, gpointer current)
{
    return current != NULL ? g_list_index (list, current) : -1;
}

static inline void
tbo_current_list_set (GList *list, gpointer *current, gpointer item)
{
    if (current == NULL)
        return;

    if (item == NULL)
    {
        *current = NULL;
        return;
    }

    *current = tbo_list_utils_contains (list, item) ? item : NULL;
}

static inline void
tbo_current_list_set_nth (GList *list, gpointer *current, gint nth)
{
    GList *link = g_list_nth (list, nth);

    if (current != NULL)
        *current = link != NULL ? link->data : NULL;
}

static inline gpointer
tbo_current_list_next (GList *list, gpointer *current)
{
    GList *link;

    if (current == NULL || *current == NULL)
        return NULL;

    link = tbo_list_utils_link (list, *current);
    if (link != NULL && link->next != NULL)
    {
        *current = link->next->data;
        return *current;
    }

    return NULL;
}

static inline gpointer
tbo_current_list_prev (GList *list, gpointer *current)
{
    GList *link;

    if (current == NULL || *current == NULL)
        return NULL;

    link = tbo_list_utils_link (list, *current);
    if (link != NULL && link->prev != NULL)
    {
        *current = link->prev->data;
        return *current;
    }

    return NULL;
}

static inline gpointer
tbo_current_list_first (GList *list, gpointer *current)
{
    gpointer item = list != NULL ? list->data : NULL;

    if (current != NULL)
        *current = item;

    return item;
}

#endif
