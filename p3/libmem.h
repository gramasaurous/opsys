// Graham Greving
// Header file for libmem memory allocation library

#ifndef _LIBMEM_H_INCLUDE_
#define _LIBMEM_H_INCLUDE_

#define MAX_ALLOCATORS 	10
#define BUDDY 			1
#define LIST 			0

// Data struct for allocator info
struct allocator {
	void *location;
	int scheme;
	int n_bytes;
	int md_bytes;
};

struct allocator allocator_list[MAX_ALLOCATORS];

/*
 * meminit
 * Allocates a region of memory of size n_bytes, initializes
 * memory management structures according to flags, parm1 and parm2
 * Possible flags: 0x1 for Buddy Allocator Style. If so, use parm1 to
 * identify the minimum block size
 * Returns a handle identifying the associated memory allocator
 */
int meminit(long n_bytes, unsigned int flags, int parm1, int parm2);

/*
 * memalloc
 * Allocates a region of n_bytes within the memory allocator
 * associated with handle.
 * Returns pointer to the allocated region
 */
void *memalloc(int handle, long n_bytes);

/*
 * meminit
 * Frees an allocated region in memory. It is up to memfree to 
 * determine which handle this region is associated with.
 */
void memfree(void *region);

#endif