#include <stdlib.h>
#include <stdio.h>

#include "libmem.h"

int main (void) {
	// Initalize a buddy scheme allocator with:
	//		256 byte size
	//		2^11 (2K) min block size
	int i;
	for (i = 0; i <= MAX_ALLOCATORS; i++) {
		int handle = meminit(256, 0x1, 11, -1);
		printf("handle: %d\n", handle);
	}
}
