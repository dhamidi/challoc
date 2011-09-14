#include "challoc.h"
#include <stdlib.h>

typedef struct chunk_allocator ChunkAllocator;
struct chunk_allocator {
     unsigned char* memory;      /* challoc returns memory from here      */
     unsigned char** chunks;     /* stack of free locations in memory     */
     size_t n_chunks;            /* number of chunks this allocator holds */
     size_t chunk_size;          /* size of single chunk in bytes         */
     size_t current_chunk;       /* stack pointer in chunks               */
     ChunkAllocator* next;       /* next allocator, if this one is full   */
};


static
void push_chunk_to_first_free_stack(ChunkAllocator* root,void* p) {
     ChunkAllocator* cur = root;

     /* find an allocator where some memory is used */
     while (cur && cur->current_chunk == cur->n_chunks)
          cur = cur->next;

     cur->chunks[++cur->current_chunk] = p;
}

static
ChunkAllocator* get_first_allocator_with_free_chunk(ChunkAllocator* root) {
     ChunkAllocator* cur = root;
     ChunkAllocator* prev = NULL;

     /* find first allocator with free memory */
     while (cur && cur->current_chunk == 0) {
          prev = cur;
          cur = cur->next;
     }

     /* all allocators have no memory left, create a new one */
     if (cur == NULL) {
          /* next allocator gets double number of chunks */
          prev->next = init_alloc(prev->n_chunks * 2, prev->chunk_size);
          return prev->next;
     }
     else
          return cur;
     
}

void* challoc(ChunkAllocator* root) {
     if (!root)
          return NULL;

     root = get_first_allocator_with_free_chunk(root);
     
     return root->chunks[root->current_chunk--];
}


void chfree(ChunkAllocator* root,void* p) {
     if (!root)
          return;

     /* all memory in this allocator is free already */
     if (root->current_chunk == root->n_chunks)
          push_chunk_to_first_free_stack(root,p);
     else
          root->chunks[++root->current_chunk] = p;
}

void chclear(ChunkAllocator* root) {
     if (!root)
          return;

     ChunkAllocator* cur = root;
     
     while (cur) {
          cur->current_chunk = cur->n_chunks-1;
          cur = cur->next;
     }
}

ChunkAllocator* chcreate(size_t n_chunks, size_t chunk_size) {
     ChunkAllocator* s = malloc(sizeof(*s));
     if (!s)
          goto FAIL;
     
     s->memory = malloc(n_chunks * chunk_size);
     if (!s->memory)
          goto FAIL;
     
     s->n_chunks = n_chunks;
     s->chunk_size = chunk_size;
     s->current_chunk = n_chunks - 1;
     s->next = NULL;
     
     s->chunks = malloc(n_chunks * sizeof(void*));
     if (!s->chunks)
          goto FAIL;

     /* add locations in s->memory to the stack */
     size_t i;
     for (i = 0; i < n_chunks; i++) {
          s->chunks[i] = &s->memory[i * chunk_size];
     }

     return s;

FAIL:
     fprintf(stderr,"init_alloc(): Out of memory\n");
     return NULL;
}

void chdestroy(ChunkAllocator** root) {
     ChunkAllocator *cur, *next;
     cur = *root;
     
     while (cur) {
          next = cur->next;
          free(cur->chunks);
          free(cur->memory);
          free(cur);
          cur = next;
     }

     *root = NULL;
}
