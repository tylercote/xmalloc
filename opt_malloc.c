// Some starter code from the ppt slides shown in class was used as inspiration for how to design
// data structs, and allocate a bin of the proper size

#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "opt_malloc.h"

//typedef struct node {
//	bool free;
//	int size;
//	struct node * next;
//	struct node * prev;
//} node;

__thread node* bins[8] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void
init_bins()
{
    int base = 5; // bins start at 32 bytes -> 4096 bytes
    for (int i = 0; i < 8; i++) {
        int bin_size = pow(2, base);
        node* start = mmap(NULL, 4096,	PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        start->next = NULL;
        start->prev = NULL;
        start->size = 4096 - sizeof(node);
        bins[i] = start;
        for (int c = 0; c < (4096 / bin_size); c++) {
            node* child = ((void*) bins[i]) - bin_size;
            child->next = NULL;
            child->prev = NULL;
            child->size = 32 - sizeof(node);
            child->free = true;
            insert_node(bins[i], child);
        }
        base++;
    }
    print_bins(bins);
}

void
print_bins() 
{
    printf("====BIN READOUT====\n");
    node* bins_copy = bins;
    for (int i = 0; i < 8; i++) {
        node* head = &bins_copy[i];
        printf("BIN SIZE = %d\n", pow(2, i + 5));
        int c = 0;
        while (head->next != NULL) {
            printf("Node %f\n", c);
            printf("Free %s\n", head->free ? "true" : "false");
            head = head->next;
            c++;
        }
    }
}

void
insert_node(node* head, node* child) {

    // adjust the size of the head node
    head->size = head->size - child->size;
    
    while (head->next != NULL) {
        head = head->next;
    }
    
    head->next = child;
    child->prev = head;
}

void*
opt_malloc(size_t usize)
{
    if (bins == 0) {
        init_bins();
    }
    
    int alloc_size = (int) usize + sizeof(node);
    // sizeof(size_t)?
    
    if (alloc_size > 4096) {
        //mmap?
    }
    else {
        alloc_size = pow_of_two(alloc_size);
    }

    // space for size
    //int64_t alloc_size = size + sizeof(int64_t);

    
    // space for free cell when returned to list
    if (alloc_size < CELL_SIZE) {
        alloc_size = CELL_SIZE;
    }
    
    pthread_mutex_lock(&mutex);
    node* empty_bin = find_empty_bin((int) usize);
    pthread_mutex_unlock(&mutex);
    void* v_empty_bin = (void*) empty_bin;
    return v_empty_bin + sizeof(node);
    
    /**
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
     * */
}

void set_free(void* addr) {
    for (int i = 0; i < 8; i++) {
        node* h = bins[i];
        while ((h->next != (node*) addr) && (h->next != NULL)) {
            h = h->next;
        }
        if (h->next == NULL) {
            continue;
        } else {
            h->free = true;
        }
    }
}
    


void opt_free(void* addr) {

    
    pthread_mutex_lock(&mutex);
    node* to_free = (node*) addr;
    to_free->free = true;
    //(node*) addr->free = true;
    pthread_mutex_unlock(&mutex);
    // do we need coalescing?
    
    /**
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
     */
}

void* opt_realloc(void* prev, size_t bytes) {
    void* ptr = opt_malloc(bytes);
    //ptr = ptr - (void*) sizeof(int64_t);
    memcpy(ptr, prev, bytes);
    opt_free(prev);
    return ptr;
}

node*
find_empty_bin(int size)
{
    node* ptr;
    int count = 0;
    bool free_bin_found = false;
    
    while (!free_bin_found) {
        int ind = log2(size) - 5;
        node* head = bins[ind];
        while (head->next != NULL) {
            if (head->free) {
                head->free = false;
                ptr = (node*) (count * size);
                free_bin_found = true;
                return ptr;
            }
            count++;
        }
        size = size * 2;
        if (size > 4096) {
            ptr = NULL;
            ptr->free = NULL;
            ptr->next = NULL;
            ptr->prev = NULL;
            ptr->size = NULL;
            return ptr;
        }
    }
    return (node*) 0xDEADBEEF;
}

int pow_of_two(int num) {
    int prev = 0;
    int curr = 0;
    int c = 0;
    while (true) {
        curr = pow((double) 2, (double) c);
        if (prev < num && num <= curr) {
            return curr;
        }
        c += 1;
        prev = curr;
    }
}

