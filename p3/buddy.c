// Graham Greving
// buddy.c

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "libmem.h"

int check_power (int n_bytes) {
	int exponent = 0;
	while ( ((n_bytes & 1) == 0 ) &&  n_bytes > 1) {
		n_bytes >>= 1;
		exponent++;
	}
	if (n_bytes == 1) {
		// plus 3 to account for bytes->bits conversion
		return (exponent+3);
	} else {
		return (-1);
	}
}

// Initializes an allocator for n_bytes with the buddy scheme
// minimum block size is determined by second parameter
// would be called by a function meminit within libmem
// min_block_size is passed in address bits 12 -> 2^12 -> 4K
int buddy_init (int n_bytes, int min_block_size) {
	// Check if n_bytes is a power of two
	int size_exponent = check_power (n_bytes);
	if (size_exponent == -1) return (-1);

	// Check that min block size isn't greater than memsize
	if (min_block_size > size_exponent) return (-1);

	// determine size of memory management: 2^(k-n)-1
	int metadata = size_exponent - min_block_size;
	int metadata_bytes = 1;
	metadata_bytes <<= metadata;

	static int handle = 0;
	if (handle >= MAX_ALLOCATORS) return (-1);

	// malloc n_bytes + metadata_size
	allocator_list[handle].location = malloc(n_bytes + metadata_bytes);
	allocator_list[handle].n_bytes = n_bytes;
	allocator_list[handle].md_bytes = metadata_bytes;
	allocator_list[handle].scheme = BUDDY;
	handle ++;
	return (handle - 1);
}


void *buddy_alloc (int handle, int n_bytes) {

}