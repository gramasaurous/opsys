/* Graham Greving
 * CMPS 111:OS - Project 4: Minix EFS
 * set.c
 * set the current users key
 */

#include <stdio.h>
#include <stdlib.h>

int main (void) {
	if (setkey(129,512) < 0) {
		printf("error\n");
	}
}
