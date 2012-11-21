/*
 * (C) Copyright 2011, Dario Hamidi <dario.hamidi@gmail.com>
 *
 * This file is part of Challoc.
 * 
 * Challoc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Challoc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Challoc.  If not, see <http://www.gnu.org/licenses/>.
 */ 
#include "challoc.h"
#include <stdlib.h>

struct chunk_allocator {
     size_t n_chunks;            /* number of chunks this allocator holds */
     size_t chunk_size;          /* size of single chunk in bytes         */
     size_t current_chunk;       /* stack pointer in chunks               */
     ChunkAllocator* next;       /* next allocator, if this one is full   */
     unsigned char** chunks;     /* stack of free locations in memory     */
     unsigned char* memory;      /* challoc returns memory from here      */
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

     /*
      * because size_t is unsigned, there is no way of telling whether
      * the memory at cur->memory[0] has been given away already or not
      * (in that case we would have to check for `cur->current_chunk == -1',
      *  which is impossible).
      */
     
     /* find first allocator with free memory */
     while (cur && cur->current_chunk == 0) {
          prev = cur;
          cur = cur->next;
     }

     /* all allocators have no memory left, create a new one */
     if (cur == NULL) {
          /* next allocator gets double number of chunks */
          prev->next = chcreate(prev->n_chunks * 2, prev->chunk_size);
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
     ChunkAllocator* cur;

     if (!root)
          return;

     cur = root;
     
     while (cur) {
          cur->current_chunk = cur->n_chunks-1;
          cur = cur->next;
     }
}

ChunkAllocator* chcreate(size_t n_chunks, size_t chunk_size) {
     ChunkAllocator* s = NULL;
     size_t i;

     /* calculate total required memory */
     size_t required_memory
          = n_chunks * chunk_size             /* space for s->memory */
          + n_chunks * sizeof(void*)          /* space for s->chunks */
          + sizeof(struct chunk_allocator);   /* space for s         */

     s = (ChunkAllocator*)malloc(required_memory);

     if (!s)
          goto FAIL;

     s->chunks = (unsigned char**)((unsigned char*)s + sizeof(*s));
     s->memory = (unsigned char*)s + sizeof(*s) + n_chunks * sizeof(void*);
     
     s->n_chunks = n_chunks;
     s->chunk_size = chunk_size;
     s->current_chunk = n_chunks - 1;
     s->next = NULL;
     
     /* add locations in s->memory to the stack of free chunks */
     for (i = 0; i < n_chunks; i++) {
          s->chunks[i] = &s->memory[i * chunk_size];
     }

     return s;

FAIL:
     return NULL;
}

void chdestroy(ChunkAllocator** root) {
     ChunkAllocator *cur, *next;
     cur = *root;
     
     while (cur) {
          next = cur->next;

          free(cur);

          cur = next;
     }

     *root = NULL;
}

/**
 * TEST
 * has to be included here, otherwise the internals of
 * struct chunk_allocator are not accessible.
 */

#ifdef CHALLOC_TEST
#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv) {
     /* arbitrary list */
     struct point_list {
          int x;
          int y;
          struct point_list *next, *prev;
     } *head, *cur, *next;

     const size_t NODE_COUNT = 100;
     size_t i;
     ChunkAllocator* nodes = chcreate(NODE_COUNT,sizeof(struct point_list));

     assert(nodes                                             );
     assert(nodes->current_chunk == NODE_COUNT - 1            );
     assert(nodes->n_chunks      == NODE_COUNT                );
     assert(nodes->chunk_size    == sizeof(struct point_list) );
     assert(nodes->next          == NULL                      );

     head = challoc(nodes);
     assert(head);

     head->x = 1;
     head->y = 1;
     head->prev = NULL;
     head->next = NULL;
     
     cur = head;
     next = NULL;

     /* create nodes and do something with them */
     for ( i = 0; i < NODE_COUNT; i++) {
          next = challoc(nodes);

          next->x = cur->x;
          if (cur->prev)
               next->x += cur->prev->x;
          next->y = i;
          
          cur->next = next;
          next->prev = cur;

          cur = next;
     }

     /*
      * by now `nodes' should have created another buffer
      * of double size.
      */

     assert(nodes->next                                      );
     assert(nodes->next->current_chunk == 2 * NODE_COUNT - 3 );
     assert(nodes->next->n_chunks      == 2 * NODE_COUNT     );
     assert(nodes->next->chunk_size    == nodes->chunk_size  );
     assert(nodes->next->next          == NULL               );

     chdestroy(&nodes);

     assert(nodes == NULL);
     return 0;
}
#endif /* CHALLOC_TEST */
