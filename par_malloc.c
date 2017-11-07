

#include <stdlib.h>
#include <unistd.h>

#include "xmalloc.h"
#include "hmem.h"


void*
xmalloc(size_t bytes)
{
    //return opt_malloc(bytes);
    void* ptr = hmalloc(bytes);
    return ptr;
}

void
xfree(void* ptr)
{
    //opt_free(ptr);
    hfree(ptr);
}

void*
xrealloc(void* prev, size_t bytes)
{
    //return opt_realloc(prev, bytes);
    void* ptr = hrealloc(prev, bytes);
    return ptr;
}

