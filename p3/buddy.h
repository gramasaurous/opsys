#ifndef _BUDDY_H_INCLUDE
#define	_BUDDY_H_INCLUDE_

int buddy_init (int n_bytes, int min_block_size);

void *buddy_alloc (int handle, int n_bytes);

#endif