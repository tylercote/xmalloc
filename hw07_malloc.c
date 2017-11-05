

#include <stdio.h>
#include <string.h>

#include "xmalloc.h"
#include "hmem.h"

/* CH02 TODO:
 *  - This should call / use your HW07 alloctor,
 *    modified to be thread-safe and have a realloc function.
 */

void*
xmalloc(size_t bytes)
{
    void* ptr = hmalloc(bytes);
    return ptr;
}

void
xfree(void* ptr)
{
    hfree(ptr);
}

void*
xrealloc(void* prev, size_t bytes)
{
    void* ptr = hrealloc(prev, bytes);
    return ptr;
}

