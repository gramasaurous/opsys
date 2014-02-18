#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "libmem.h"
#include "buddy.h"

int meminit(long n_bytes, unsigned int flags, int parm1, int parm2) {
	// Buddy scheme
	if  (flags == 0x1) {
		int handle = buddy_init(n_bytes, parm1);
		return (handle);
	} else {
		return (0);
	}
}

// void *memalloc(int handle, long n_bytes) {
// }

// void memfree(void *region) {
// 	return;
// }

int memdump() {
	int i;
	for (i = 0; i < MAX_ALLOCATORS; i++) {
		struct allocator curr = allocator_list[i];
		printf("allocator: %d\n", i);
		printf("handle: %d\n", curr.location);
		printf("handle: %d\n", curr.scheme);
		printf("handle: %d\n", curr.bytes);
		printf("handle: %d\n", curr.md_bytes);
	}
}