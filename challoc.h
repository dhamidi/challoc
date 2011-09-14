/* Challoc - chunk allocator
 *   Memory allocator for efficiently handling lots of objects
 *   of the same size.
 *
 * (C) Copyright 2011, Dario Hamidi <dario.hamidi@gmail.com>
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
#ifndef CHALLOC_H
#define CHALLOC_H

#include <stddef.h>

/*
 * challoc - chunk allocator
 * Creates an object that allocates a large buffer for allocations
 * of many same sized objects. The buffer can be used to reuse
 * allocated objects as well as to free the memory used for them
 * all at once.
 *
 * How it works:
 * When a challoc object is created, it allocates an internal buffer
 * large enough for a given number of objects of a given size. Objects
 * can then be allocated and deallocated from this buffer, effectively
 * reducing the number of calls to malloc(). If more objects are
 * requested than the buffer can hold at the moment, another buffer is
 * allocated and linked to the first one. This list of buffers has to be
 * searched every time a new object is allocated or deallocated from that
 * buffer. Every time a new buffer is created, the amount of objects it
 * can hold is doubled, to minimize the length of the buffer list.
 *
 * How to use it:
 * First create a chunk allocator by calling chcreate(). The number
 * of chunks (i.e. objects) should be a rough estimate of how many objects
 * you need - in case of doubt, use a larger number.
 * After having created a ChunkAllocator object this way, allocate your
 * objects with challoc() and deallocate them with chfree().
 * If you want to discard all objects from a ChunkAllocator use chclear().
 * This does not return any memory to the operating systems, it simply
 * marks the buffer as "empty".
 * When you want to return the memory to the operating system, call
 * chdestroy(). Note that all objects allocated by this ChunkAllocator
 * become invalid by using that operation.
 *
 * When to use it:
 * This allocating scheme works best when you need to frequently allocate
 * a large number of objects of the same size. This is almost always the
 * case when dealing with collection objects such as hash tables or linked
 * lists.
 *
 * Why use it:
 * It can increase performance. By reusing the same memory area, heap fragment-
 * ation is slowed, locality of reference is kept and the overhead of frequent
 * calls to malloc()/free() is avoided.
 */

typedef struct chunk_allocator ChunkAllocator;

/*
 * These functions deal with the system's memory!
 */

/* Create a ChunkAllocator for `n' elements of size `size' */
ChunkAllocator* chcreate(size_t n, size_t size);

/*
 * Destroy a ChunkAllocator returning the memory to the OS.
 * ch will be NULL after a call to chdestroy.
 */
void chdestroy(ChunkAllocator** ch);

/*
 * These functions deal with a ChunkAllocator's buffer.
 */

/*
 * Get memory for an object from ChunkAllocator `ch'.
 * Returns NULL if no memory is available anymore.
 */
void* challoc(ChunkAllocator* ch);

/* Mark the object `o' as free in the ChunkAllocator's buffer */ 
void chfree(ChunkAllocator* ch, void* o);

/* Mark all objects in the ChunkAllocator's buffer as free */
void chclear(ChunkAllocator* ch);

#endif
