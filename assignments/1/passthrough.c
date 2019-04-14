/**
 * @file   passthrough.c
 * @brief  An allocator that passes calls through to libc's `malloc(3)`, etc.,
 *         keeping track of the results to be able to answer questions about
 *         outstanding allocations.
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
#include "rtos-alloc.h"


void*
rtos_malloc(size_t size)
{
	if (alloc_debugging())
	{
		printf("\nAllocating %zu B... ", size);
	}

	void *p = malloc(size);
	if (p == NULL)
	{
		return NULL;
	}

	alloc_t *ap = alloc_new(p, size);
	if (ap == NULL)
	{
		free(p);
		return NULL;
	}

	if (alloc_debugging())
	{
		alloc_print_all();
	}

	return p;
}

void*
rtos_realloc(void *p, size_t size)
{
	if (alloc_debugging())
	{
		printf("Reallocating %p -> %zu B... ", p, size);
	}

	alloc_t *ap = alloc_find(p);
	if (ap == NULL)
	{
		return NULL;
	}

	void *newp = realloc(p, size);
	if (newp != p)
	{
		ap->a_base = newp;
	}

	ap->a_len = size;

	if (alloc_debugging())
	{
		alloc_print(ap);
	}

	return newp;
}

void
rtos_free(void *p)
{
	if (alloc_debugging())
	{
		printf("\nRemoving %p from ", p);
		alloc_print_all();
	}

	if (p == NULL)
	{
		return;
	}

	alloc_t *ap = alloc_find(p);
	assert(ap != NULL);

	alloc_remove(ap);
	free(p);

	if (alloc_debugging())
	{
		printf("Resulting ");
		alloc_print_all();
	}
}

size_t
rtos_alloc_size(void *p)
{
	alloc_t *ap = alloc_find(p);
	assert(ap != NULL);

	return ap->a_len;
}

bool
rtos_is_valid(void *p)
{
	return alloc_find(p);
}

size_t
rtos_total_allocated()
{
	size_t total = 0;

	for (alloc_t *ap = alloc_first(); ap != NULL; ap = ap->a_next)
	{
		total += ap->a_len;
	}

	return total;
}
