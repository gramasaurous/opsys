/* Graham Greving
 * CMPS111: Project 4
 * do_setkey.c
 * do_setkey system call handler
 * Creates an AES encryption key for the current user with the defined integers
 */

#include "fs.h"
#include "fproc.h"
#include "lib.h"

PUBLIC int do_setkey(void) {

	unsigned int k0 = m_in.m1_i1;
	unsigned int k1 = m_in.m1_i2;
	uid_t uid = fp->fp_realuid;

	printf("kernel: do_setkey(%d, %d)\n",k0,k1);
	printf("kernel: uid:%d\n", (int) uid);
	
}