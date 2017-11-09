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
__thread node* headx = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void
init_bins()
{
    printf("Enter init_bins\n");
    int base = 5; // bins start at 32 bytes -> 4096 bytes
    for (int i = 0; i < 8; i++) {
        int bin_size = pow(2, base);
        printf("Generating bins of size: %d\n", bin_size);
        node* start = mmap(NULL, 4096,	PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        start->next = NULL;
        start->prev = NULL;
        start->free = false;
        //start->size = 4096 - sizeof(node);
        void* ptr = (void*) start;
        for (int c = 0; c < (4096 / bin_size) + 1; c++) {
            printf("**bin num: %d\n", c);
            //node* child = ((void*) bins[i]) - bin_size;
            node* prev_node = (node*) ptr;
            ptr += bin_size;
            node* child = (node*) ptr;
            child->next = NULL;
            child->prev = prev_node;
            prev_node->next = child;
            //if (start->next == NULL) {
            //    printf("STILL NULL\n");
            //}
            //insert_node(bins[i], child); // fixers prev and next
            //child->next = NULL;
            child->size = 32;
            child->free = true;
        }
        bins[i] = start;
        base++;
    }
    printf("Finished init_bins\n");
    //print_bins(bins);
}

void
init_bin()
{
    node* h = headx;
    h = mmap(NULL, 4096,	PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
    h->next = NULL;
    h->size = 4096 - sizeof(node);
    for(int i = 0; i < 128; i++) {
        while(h != NULL) {
            h = h->next;
        }
        if (h->next == NULL) {
            node* c = (void*) h - 32;
            c->next = NULL;
            c->size = h->size - 32 - sizeof(node);
            h->next = c;
        }
        
    }
    printf("node added");
}

void
print_bins() 
{
    printf("====BIN READOUT====\n");
    //node* bins_copy = bins;
    for (int i = 0; i < 8; i++) {
        node* head = bins[i];
        double bin_size = pow(2, i + 5);
        printf("BIN SIZE = %f\n", bin_size);
        int c = 0;
        while (head->next != NULL) {
            printf("Node: %d\n", c);
            printf("Free: %s\n", head->free ? "true" : "false");
            c++;
            head = head->next;
        }
    }
}

void
count_bins()
{
        node* h = headx;
        int c = 0;
        while (h->next != NULL) {
            c++;
            h = h->next;
            printf("%i\n", c);
        }
        printf("add one to c");
}

void
insert_node(node* head, node* child) {
    //printf("Enter insert_node\n");
    // adjust the size of the head node
    
    head->size = head->size - child->size;
    node* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    
    temp->next = child;
    child->prev = temp;
    child->next = NULL;
}

void*
opt_malloc(size_t usize)
{
    printf("Enter opt_malloc\n");
    if (bins[0] == NULL) {
        init_bin();
        printf("Bins initialized\n");
        //count_bins();
        //print_bins();
    }
    else {
        printf("Bins initialized already\n");
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
    
    printf("Before finding empty bin\n");
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

