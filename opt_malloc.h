#ifndef OPT_MALLOC_H
#define OPT_MALLOC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct node {
	bool free;
	int size;
	struct node * next;
	struct node * prev;
} node;

//static const int CHUNK_SIZE = 65536;
static const int CELL_SIZE  = (int)sizeof(node);

void init_bins();

void print_bins();

void* opt_malloc(size_t usize);

void opt_free(void* addr);

void* opt_realloc(void* prev, size_t bytes);

void insert_node(node* head, node* child);

node* find_empty_bin(int size);

#endif // OPT_MALLOC_H
