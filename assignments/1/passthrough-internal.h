/**
 * @file   passthrough-internal.h
 * @brief  Declarations of internal types and functions for the passthrough
 *         allocator.
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

#include <stdbool.h>
#include <stdlib.h>

/**
 * A record of an allocation that we've received from `malloc(3)`.
 */
typedef struct allocation {
	void	*a_base;
	size_t	a_len;

	struct allocation *a_prev;
	struct allocation *a_next;
} alloc_t;


bool			alloc_debugging(void);
struct allocation*	alloc_find(void *p);
struct allocation*	alloc_first(void);
struct allocation*	alloc_init(void);
struct allocation*	alloc_new(void *p, size_t len);
void			alloc_print(struct allocation *ap);
void			alloc_print_all(void);
void			alloc_remove(struct allocation *ap);
