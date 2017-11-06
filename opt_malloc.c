// Some starter code from the ppt slides shown in class was used as inspiration for how to design
// data structs, and allocate a bin of the proper size

#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#include "op_alloc.h"

//typedef struct node {
//	bool free;
//	int size;
//	struct node * next;
//	struct node * prev;
//} node;

//typedef struct header {
//	bool free;
//	int size;
//} header;

//typedef struct footer {
//	int size;
//} header;

static __thread node* bins[7] = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int64_t
nu_free_list_length()
{
    int len = 0;

    for (nu_free_cell* pp = nu_free_list; pp != 0; pp = pp->next) {
        len++;
    }

    return len;
}

void
nu_print_free_list()
{
    nu_free_cell* pp = nu_free_list;
    printf("= Free list: =\n");

    for (; pp != 0; pp = pp->next) {
        printf("%lx: (cell %ld %lx)\n", (int64_t) pp, pp->size, (int64_t) pp->next); 

    }
}

static
void
nu_free_list_coalesce()
{
    nu_free_cell* pp = nu_free_list;
    int free_chunk = 0;

    while (pp != 0 && pp->next != 0) {
        if (((int64_t)pp) + pp->size == ((int64_t) pp->next)) {
            pp->size += pp->next->size;
            pp->next  = pp->next->next;
        }

        pp = pp->next;
    }
}

static
void
nu_free_list_insert(nu_free_cell* cell)
{
    if (nu_free_list == 0 || ((uint64_t) nu_free_list) > ((uint64_t) cell)) {
        cell->next = nu_free_list;
        nu_free_list = cell;
        return;
    }

    nu_free_cell* pp = nu_free_list;
    
    while (pp->next != 0 && ((uint64_t)pp->next) < ((uint64_t) cell)) {
        pp = pp->next;
    }

    cell->next = pp->next;
    pp->next = cell;

    nu_free_list_coalesce();
}

static
nu_free_cell*
free_list_get_cell(int64_t size)
{
    nu_free_cell** prev = &nu_free_list;

    for (nu_free_cell* pp = nu_free_list; pp != 0; pp = pp->next) {
        if (pp->size >= size) {
            *prev = pp->next;
            return pp;
        }
        prev = &(pp->next);
    }
    return 0;
}

static
nu_free_cell*
make_cell()
{
    void* addr = mmap(0, CHUNK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    nu_free_cell* cell = (nu_free_cell*) addr; 
    cell->size = CHUNK_SIZE;
    return cell;
}

void
init_bins()
{
    int base = 5; // bins start at 32 bytes -> 1024 bytes
    for (int i = 0; i < 7; i++) {
        
        node* start
        bins[i] = 
    }
}

void*
opt_malloc(size_t usize)
{
    int size = (int) usize;
    size += sizeof(header) + sizeof(footer);
    if (size > 2048) {
        size = 4096 * ((size + 4095) / 4096);
    }
    else if (size < 128) {
        size = 32 * ((size + 31) / 32);
    }
    else {
        size = pow_of_two(size);
    }


    // space for size
    int64_t alloc_size = size + sizeof(int64_t);

    // space for free cell when returned to list
    if (alloc_size < CELL_SIZE) {
        alloc_size = CELL_SIZE;
    }

    // TODO: Handle large allocations.
    if (alloc_size > CHUNK_SIZE) {
        void* addr = mmap(0, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        *((int64_t*)addr) = alloc_size;
        //nu_malloc_chunks += 1;
        return addr + sizeof(int64_t);
    }

    nu_free_cell* cell = free_list_get_cell(alloc_size);
    if (!cell) {
        cell = make_cell();
    }

    // Return unused portion to free list.
    int64_t rest_size = cell->size - alloc_size;
    if (rest_size >= CELL_SIZE) {
        void* addr = (void*) cell;
        nu_free_cell* rest = (nu_free_cell*) (addr + alloc_size);
        rest->size = rest_size;
        pthread_mutex_lock(&mutex);
        nu_free_list_insert(rest);
        pthread_mutex_unlock(&mutex);
    }

    *((int64_t*)cell) = alloc_size;
    return ((void*)cell) + sizeof(int64_t);
}

void
hfree(void* addr) 
{
    nu_free_cell* cell = (nu_free_cell*)(addr - sizeof(int64_t));
    int64_t size = *((int64_t*) cell);

    if (size > CHUNK_SIZE) {
        //nu_free_chunks += 1;
        munmap((void*) cell, size);
    }
    else {
        cell->size = size;
        pthread_mutex_lock(&mutex);
        nu_free_list_insert(cell);
        pthread_mutex_unlock(&mutex);
    }
}

void*
hrealloc(void* prev, size_t bytes) {
    
    //int64_t size = (int64_t) bytes;

    // space for size
    //int64_t alloc_size = size + sizeof(int64_t);
    
    void* ptr = hmalloc(bytes);
    //ptr = ptr - (void*) sizeof(int64_t);
    memcpy(ptr, prev, bytes);
    hfree(prev);
    return ptr;
    
    //*((int64_t*)ptr) = alloc_size;
    //return ((void*)ptr) + sizeof(int64_t);
}

int pow_of_two(int num) {
    int prev = 0;
    int curr = 0;
    int c = 0;
    while (true) {
        curr = pow(2, c);
        if (prev < num && num <= curr) {
            return curr;
        }
        c += 1;
        prev = curr;
    }
}

