#ifndef OP_ALLOC_H
#define OP_ALLOC_H

#include <stdint.h>

typedef struct node {
	bool free;
	int size;
	struct node * next;
	struct node * prev;
} node;

typedef struct header {
	bool free;
	int size;
} header;

typedef struct footer {
	int size;
} header;

static const int CHUNK_SIZE = 65536;
static const int CELL_SIZE  = (int)sizeof(node);

int free_list_length();

void print_free_list();

static void free_list_coalesce();

static void free_list_insert(nu_free_cell* cell);

static nu_free_cell* free_list_get_cell(int64_t size);

static nu_free_cell* make_cell();

void* op_malloc(size_t usize);

void op_free(void* addr);

//void* realloc(void* prev, size_t bytes);

#endif // OP_ALLOC_H
