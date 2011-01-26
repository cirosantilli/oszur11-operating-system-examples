/*! List manipulation functions
 *
 * Double linked lists are used
 * List header points to first and last list element
 *
 * List manipulation is implemented through macros
 * (look below for example usage)
 */

#include "list.h"

#include <lib/types.h>
#include <kernel/errno.h>

void list_init ( list_t *list )
{
	ASSERT ( list );

	list->first = list->last = NULL;
}

/*! Add element to list, add to tail - as last element */
void list_append ( list_t *list, void *object, list_h *hdr )
{
	ASSERT ( list && object && hdr );

	hdr->object = object; /* save reference to object */
	hdr->next = NULL; /* put it at list end (as last element) */

	if ( list->first )
	{
		list->last->next = hdr;
		hdr->prev = list->last;
		list->last = hdr;
	}
	else {
		list->first = list->last = hdr;
		hdr->prev = NULL;
	}
}

/*! Add element to list, add to head - as first element */
void list_prepend ( list_t *list, void *object, list_h *hdr )
{
	ASSERT ( list && object && hdr );

	hdr->object = object; /* save reference to object */
	hdr->prev = NULL; /* put it at list start (as first element) */

	hdr->next = list->first;

	if ( list->first )
		list->first->prev = hdr; /* list was not empty */
	else
		list->last = hdr; /* list was empty */

	list->first = hdr;
}

/*! Add element to sorted list */
void list_sort_add ( list_t *list, void *object, list_h *hdr,
				   int (*cmp) ( void *, void * ) )
{
	list_h *iter;

	ASSERT ( list && object && hdr && cmp );

	hdr->object = object; /* save reference to object */

	/* speedup search if elem is to be last in list */
	if ( list->last && cmp ( object, list->last->object ) >= 0 )
		iter = NULL; /* insert after last */
	else
		iter = list->first; /* start searching from beginning */

	while ( iter && cmp ( object, iter->object ) >= 0 )
		iter = iter->next;

	/* insert before 'iter' */
	hdr->next = iter;

	if ( iter )
	{
		hdr->prev = iter->prev;
		if ( iter->prev )
			iter->prev->next = hdr;
		else
			list->first = hdr; /* 'iter' was first in list */
		iter->prev = hdr;
	}
	else {
		/* 'iter' == NULL => add to the end of list */
		if ( list->last )
		{
			/* list was not empty */
			hdr->prev = list->last;
			list->last->next = hdr;
			list->last = hdr;
		}
		else {
			/* list was empty */
			list->first = list->last = hdr;
			hdr->prev = NULL;
		}
	}
}

/*! Get pointer to first or last list element */
void *list_get ( list_t *list, unsigned int flags )
{
	list_h *hdr;

	ASSERT ( list );

	if ( flags & LAST )
		hdr = list->last;
	else
		hdr = list->first;

	if ( hdr )
		return hdr->object;
	else
		return NULL;
}

/*! Get pointer to next object in list */
void *list_get_next ( list_h *hdr )
{
	if ( !hdr || !hdr->next )
		return NULL;
	else
		return hdr->next->object;
}

/*!
 * Remove element from list
 * \param list	List identifier (pointer)
 * \param flags	Constant: FIRST(0) or LAST(1) - which element to remove from
 *		list and return pointer to it
 * \param ref	Reference (pointer) to element to be removed from list
 * \returns pointer to removed list element, NULL if list is empty
 */
void *list_remove ( list_t *list, unsigned int flags, list_h *ref )
{
	list_h *hdr;

	ASSERT ( list );

	if ( ref )
		hdr = ref;
	else if ( flags & LAST )
		hdr = list->last;
	else
		hdr = list->first;

	if ( hdr )
	{
		if ( hdr->prev )
			hdr->prev->next = hdr->next;

		if ( hdr->next )
			hdr->next->prev = hdr->prev;

		if ( list->first == hdr )
			list->first = hdr->next;

		if ( list->last == hdr )
			list->last = hdr->prev;

		return hdr->object;
	}
	else {
		return NULL;
	}
}
