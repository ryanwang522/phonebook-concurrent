#include <stddef.h> // for offsefof

typedef struct __LIST_HEAD {
    struct __LIST_HEAD *next, *prev;
} list_t;

/* Declr an empty list head */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

inline
void INIT_LIST_HEAD(list_t *list)
__attribute__((always_inline));

inline
int list_empty(const list_t *head)
__attribute__((always_inline));

inline
void __list_add(list_t *new, list_t *prev, list_t *next)
__attribute__((always_inline));

inline
void list_add_tail(list_t *new, list_t *head)
__attribute__((always_inline));

inline
void list_add(list_t *new, list_t *head)
__attribute__((always_inline));

inline
void __list_splice(const list_t *list, list_t *prev, list_t *next)
__attribute__((always_inline));

inline
void list_splice(const list_t *list, list_t *head)
__attribute__((always_inline));

inline
void list_splice_tail(list_t *list, list_t *head)
__attribute__((always_inline));

inline
void INIT_LIST_HEAD(list_t *list)
{
    list->next = list;
    list->prev = list;
}

inline
int list_empty(const list_t *head)
{
    return head->next == head;
}

inline
void __list_add(list_t *new, list_t *prev, list_t *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/* Push to linked list's head (stack) */
inline
void list_add(list_t *new, list_t *head)
{
    __list_add(new, head, head->next);
}

inline
void list_add_tail(list_t *new, list_t *head)
{
    __list_add(new, head->prev, head);
}

inline
void __list_splice(const list_t *list, list_t *prev, list_t *next)
{
    list_t *first = list->next;
    list_t *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
inline
void list_splice(const list_t *list, list_t *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head, head->next);
    }
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
inline
void list_splice_tail(list_t *list, list_t *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head->prev, head);
    }
}

/* get a pointer pointting to `type` */
#define list_entry(ptr, type, member) \
    (type *)((char *)ptr - offsetof(type, member))

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each_entry(pos, head, member)		\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))

