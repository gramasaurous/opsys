/* Graham Greving
 * CMPS 111:OS - Project 4: Minix EFS
 * clear.c
 * clear the current users key
 */

#include <stdio.h>
#include <stdlib.h>

int main (void) {
	if (setkey(0,0) < 0) {
		printf("error\n");
	}
}
