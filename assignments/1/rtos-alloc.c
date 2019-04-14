/****************************************************
* RTOS Assignment 1
* Christian Legge
* 201422748
* January 25, 2019
****************************************************/

#include <time.h>
#include <stdio.h>
#include "rtos-alloc.h"

/**
 * Allocate @b size bytes of memory for the exclusive use of the caller,
 * as `malloc(3)` would.
 */

#define BLOCK_ALIGN 32
 
 typedef struct free_header {
   size_t length;
   struct free_header* next;
 } free_h;

free_h freelist_head = { 0, NULL };

size_t header_size = sizeof(free_h);
 
void*	rtos_malloc(size_t size) 
{
  free_h *header_ptr = &freelist_head;
  free_h *last = header_ptr;
  size_t total_size = size + header_size;
  total_size += (total_size % 32 == 0 ? 0 : total_size + (BLOCK_ALIGN - (total_size % 32)));
  while (header_ptr)
  {
      if (header_ptr->length >= total_size) 
      {
          break;
      }
      last = header_ptr;
      header_ptr = header_ptr->next;
  }
  if (!header_ptr) 
  {
      sbrk(total_size);
      last->length += total_size;
      header_ptr = last;
  }

  if (header_ptr->length - total_size > BLOCK_ALIGN)
  {
        free_h* new_header = header_ptr + total_size + header_size;
        new_header->length = total_size;
        header_ptr->next = new_header;
  }


  return header_ptr;

}

/**
 * Change the size of the allocation starting at @b ptr to be @b size bytes,
 * as `realloc(3)` would.
 */
void*	rtos_realloc(void *ptr, size_t size) 
{
    free_h* header_ptr = &freelist_head;

    while(header_ptr)
    {
        if (header_ptr + header_ptr->length == ptr)
        {
            if (header_ptr->next && header_ptr->next->length >= (header_ptr->next - header_ptr->next + size)) 
            {
                header_ptr->next = (header_ptr->next->next) ? header_ptr->next->next : NULL;
            }
            else 
            {
                return NULL;
            }
        }
        header_ptr = header_ptr->next;
    }
    return NULL;
}

/**
 * Release the memory allocation starting at @b ptr for general use,
 * as `free(3)` would.
 */
void	rtos_free(void *ptr)
{
    free_h* header_ptr = &freelist_head;

    while (header_ptr)
    {
        if (header_ptr + header_ptr->length == ptr)
        {
            header_ptr->length = header_ptr->next - header_ptr;
            if(header_ptr->next)
            {
                header_ptr->next = header_ptr->next->next;
                return;
            }
        }
        header_ptr = header_ptr->next;
    }
}


/*
 * The following functions will help our test code inspect your allocator's
 * internal state:
 */

/**
 * How large is the allocation pointed at by this pointer?
 *
 * @pre   @b ptr points at a valid allocation (according to `rtos_allocated`)
 */
size_t	rtos_alloc_size(void *ptr)
{
    free_h* header_ptr = &freelist_head;

    while (header_ptr)
    {
        if (header_ptr + header_ptr->length == ptr)
        {
            return header_ptr->next - (header_ptr + header_ptr->length);
        }
        header_ptr = header_ptr->next;
    }

    return 0;
}

/**
 * Does this pointer point at the beginning of a valid allocation?
 *
 * @param   ptr    any virtual address
 *
 * @returns whether or not @b ptr is a currently-allocated address returned
 *          from @b my_{m,re}alloc
 */
bool	rtos_allocated(void *ptr)
{
  
    free_h* header_ptr = &freelist_head;

    while (header_ptr)
    {
        if (header_ptr + header_ptr->length == ptr)
        {
            return true;
        }
        header_ptr = header_ptr->next;
    }

    return false;
}

/**
 * How much memory has been allocated by this allocator?
 *
 * @returns the number of bytes that have been allocated to user code,
 *          **not** including any allocator overhead
 */
size_t	rtos_total_allocated(void)
{
    
    free_h* header_ptr = &freelist_head;
    size_t totalsize = 0;
    while (header_ptr)
    {
        if (header_ptr->next)
        {
            totalsize += header_ptr->next - (header_ptr + header_ptr->length);
        }
        header_ptr = header_ptr->next;
    }

    return totalsize;
  
}

int main() 
{
    for (int i = 1; i <= 100; i++)
    {
        struct timespec tick, tock;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tick);
        printf("%d: \t", i*8);
        rtos_malloc(i*8);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tock);
        printf("%d\n", tock.tv_nsec - tick.tv_nsec);
    }
}
