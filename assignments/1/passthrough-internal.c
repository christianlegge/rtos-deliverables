/**
 * @file   passthrough-internal.c
 * @brief  Internal code to keep track of existing allocations by the
 *         passthrough allocated.
 */
/*
 * Copyright 2018 Jonathan Anderson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <stdio.h>
#include <strings.h>

#include "passthrough-internal.h"

/**
 * A doubly-linked list of all outstanding allocations.
 */
static struct allocation *first_alloc, *last_alloc;


bool
alloc_debugging()
{
	return getenv("DEBUG_RTOS_ALLOC");
}

struct allocation*
alloc_find(void *p)
{
	for (alloc_t *ap = first_alloc; ap != NULL; ap = ap->a_next)
	{
		assert(ap->a_base != NULL);
		if (ap->a_base == p)
		{
			return ap;
		}
	}

	return NULL;
}

struct allocation*
alloc_first()
{
	return first_alloc;
}

struct allocation*
alloc_init()
{
	return first_alloc = last_alloc = calloc(1, sizeof(struct allocation));
}

struct allocation*
alloc_new(void *p, size_t len)
{
	alloc_t *ap;

	// Either create the first allocation record or create a new one and
	// attach it to the end of the linked list.
	if (first_alloc == NULL)
	{
		ap = alloc_init();
		if (ap == NULL)
		{
			return NULL;
		}
	}
	else
	{
		ap = calloc(1, sizeof(struct allocation));
		last_alloc->a_next = ap;
		ap->a_prev = last_alloc;
		last_alloc = ap;
	}

	ap->a_base = p;
	ap->a_len = len;

	return ap;
}

void
alloc_print(struct allocation *ap)
{
	printf("{ %12p %4zu %12p %12p }\n",
	       ap->a_base, ap->a_len, ap->a_prev, ap->a_next);
}

void
alloc_print_all()
{
	printf("allocations:\n");

	for (alloc_t *ap = first_alloc; ap != NULL; ap = ap->a_next)
	{
		printf("  ");
		alloc_print(ap);

		assert(ap == last_alloc || ap->a_next != NULL);
	}
}

void
alloc_remove(struct allocation *ap)
{
	if (alloc_debugging())
	{
		printf("Removing ");
		alloc_print(ap);
	}

	/*
	 * Remove this allocation from the linked list.
	 *
	 * First find the previous and next elements (if any) in the list.
	 */
	alloc_t *prevp = ap->a_prev;
	alloc_t *nextp = ap->a_next;

	/*
	 * Fix up the list by rewriting the previous allocation's "next" pointer
	 * (or `first_alloc` if there is no previous allocation) and the next
	 * allocation's "prev" pointer (or `last_alloc` if there is no next
	 * allocation).
	 */
	alloc_t **prevnextp = (prevp ? &prevp->a_next : &first_alloc);
	*prevnextp = nextp;

	alloc_t **nextprevp = (nextp ? &nextp->a_prev : &last_alloc);
	*nextprevp = prevp;

	free(ap);
}
