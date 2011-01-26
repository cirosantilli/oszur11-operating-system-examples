/*! List manipulation functions
 *
 * Double linked lists are used
 * List header points to first and last list element
 *
 * List manipulation is implemented through macros
 * (look below for example usage)
 */

#pragma once

typedef struct _list_h_
{
	struct _list_h_ *prev;
	struct _list_h_ *next;
	void *object;
}
list_h;

#define LIST_H_NULL	{ NULL, NULL, NULL }

/*! list header type */
typedef struct _list_
{
	list_h *first;
	list_h *last;
}
list_t;

#define LIST_T_NULL	{ NULL, NULL }

#define FIRST	0	/* get first or last list element */
#define LAST	1

void list_init ( list_t *list );
void list_append ( list_t *list, void *object, list_h *hdr );
void list_prepend ( list_t *list, void *object, list_h *hdr );
void list_sort_add ( list_t *list, void *object, list_h *hdr,
				   int (*cmp) ( void *, void * ) );
void *list_get ( list_t *list, unsigned int flags );
void *list_remove ( list_t *list, unsigned int flags, list_h *ref );
void *list_get_next ( list_h *hdr );
