#include <stdbool.h>

struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD_DEFINE(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

static inline void __list_add(struct list_head *node,
    struct list_head *prev,
    struct list_head *next)
{
    prev->next = node;
    next->prev = node;
    node->prev = prev;
    node->next = next;
}

static inline void list_insert(struct list_head *head,
    struct list_head *node)
{
    __list_add(node, head, head->next);
}

static inline void list_append(struct list_head *head,
    struct list_head *node)
{
    __list_add(node, head->prev, head);
}

static inline void __list_del(struct list_head *prev,
    struct list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void list_del(struct list_head *node)
{
    __list_del(node->prev, node->next);
}

static inline void list_move(struct list_head *head,
    struct list_head *node)
{
    __list_del(node->prev, node->next);
    list_insert(head, node);
}

static inline void list_move_tail(struct list_head *head,
    struct list_head *node)
{
    __list_del(node->prev, node->next);
    list_append(head, node);
}

static inline bool list_empty(struct list_head *head)
{
    return head->next == head;
}

/* for entry */

#define list_for_each(pos, head)    \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define list_for_each_entry(pos, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member);    \
         &pos->member != (head);                     \
         pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member),    \
        n = list_entry(pos->member.next, typeof(*pos), member);    \
         &pos->member != (head);                     \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

