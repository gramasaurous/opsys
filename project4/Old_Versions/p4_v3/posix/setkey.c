/* Graham Greving
 * CMPS111: Project 4
 * setkey.c
 * Allows userspace access to the setkey() system call.
 * Creates an AES encryption key for the current user with the defined integers
 */

#include <stdio.h>
#include <stdlib.h>
#include <lib.h>
#include <unistd.h>

#define SETKEY 69

PUBLIC int setkey(unsigned int k0, unsigned int k1){
	message m;
	
	m.m1_i1 = k0;
	m.m1_i2 = k1;

	return(_syscall(FS,SETKEY,&m));
}
