/* Graham Greving
 * CMPS 111:OS - Project 4: Minix EFS
 * protectfile.c
 *
 * Encrypt or decrypt a file using AES with the users key
 * appropriately sets/clears the files stickybit
 * 
 * This program uses CTR mode encryption.
 *
 * Usage: protectfile <op> <hexkey> <file name>
 *
 * Author: Ethan L. Miller (elm@cs.ucsc.edu)
 * Based on code from Philip J. Erdelsky (pje@acm.org)
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "rijndael.h"

#define ENCRYPT_BIT 01000
#define KEYBITS 	128
#define ENCRYPT 	0
#define DECRYPT 	1

/***********************************************************************
 *
 * hexvalue
 *
 * This routine takes a single character as input, and returns the
 * hexadecimal equivalent.  If the character passed isn't a hex value,
 * the program exits.
 *
 ***********************************************************************
 */
int hexvalue (char c)
{
	if (c >= '0' && c <= '9') {
	return (c - '0');
	} else if (c >= 'a' && c <= 'f') {
	return (10 + c - 'a');
	} else if (c >= 'A' && c <= 'F') {
	return (10 + c - 'A');
	} else {
		fprintf (stderr, "ERROR: key digit %c isn't a hex digit!\n", c);
		exit (-1);
	}
}

int main(int argc, char **argv)
{
	unsigned char key[KEYLENGTH(KEYBITS)]; /* cipher key */
	unsigned long rkey[RKLENGTH(KEYBITS)]; /* round key */
	int i, fd, ctr;
	int nrounds, nbytes;
	int nwritten;
	int totalbytes;
	int op;
	char *keyin = argv[2];
	char *filename = argv[3];
	struct stat st;
	ino_t inode;
	unsigned char filedata[16];
	unsigned char ciphertext[16];
	unsigned char ctrvalue[16];
	char *op_in;

	if (argc != 4) {
		fprintf(stderr, "Useage: %s <op> <hexkey> <file>\n", argv[0]);
		return (-1);
	}
	/* Check the opcode */
	if (argv[1][0] == 'e') {
		printf("encrypt that sucker!\n");
		op = ENCRYPT;
	} else if (argv[1][0] == 'd') {
		printf("decrypt that sucker!\n");
		op = DECRYPT;
	} else {
		printf("bad op: %c\n", argv[1][0]);
		exit (-1);
	}
	/* Clean the buffers */
	bzero(key, sizeof(key));
	bzero(rkey, sizeof(rkey));
	bzero(ctrvalue, sizeof(ctrvalue));
	bzero(ciphertext, sizeof(ciphertext));
	bzero(filedata, sizeof(filedata));

	for(i = 0; i < KEYLENGTH(KEYBITS); i++) {
		key[i] = hexvalue(keyin[i]);
	}
	/* Open the file */
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Error opening file %s\n", filename);
		exit(-1);
	}
	/* Get the file details */
	if (fstat(fd, &st) < 0) {
		return (-1);
	}
	/* Check for the sticky bit */
	if ( ((st.st_mode & ENCRYPT_BIT) == 0) && (op == DECRYPT)) {
		fprintf(stderr, "Error: file already decrypted: %s\n", filename);
		printf("%d\n", st.st_mode);
		close(fd);
		exit(-1);
	} else if (((st.st_mode & ENCRYPT_BIT)) && (op == ENCRYPT)) {
		fprintf(stderr, "Error: file already encrypted: %s\n", filename);
		printf("%d\n", st.st_mode);
		close(fd);
		exit(-1);
	}
	inode = st.st_ino;
	printf("inode: %d\n", inode);
	/* Initialize the encryption */
	nrounds = rijndaelSetupEncrypt(rkey, key, KEYBITS);
	bcopy (&inode, &(ctrvalue[8]), sizeof(inode));

	/* Start the encryption loop */
	for (ctr = 0, totalbytes = 0; ; ctr++) {
		nbytes = read(fd, filedata, sizeof(filedata));

		if (nbytes <= 0) {
			break;
		}
		if (lseek(fd, totalbytes, SEEK_SET) < 0) {
			perror ("Unable to seek back over buffer");
			exit (-1);
		}
		bcopy(&ctr, &(ctrvalue[0]), sizeof(ctr));

		rijndaelEncrypt(rkey, nrounds, ctrvalue, ciphertext);

		for (i = 0; i < nbytes; i++) {
			filedata[i] ^= ciphertext[i];
		}

		nwritten = write(fd, filedata, nbytes);
		if (nwritten != nbytes) {
			fprintf(stderr, 
				"%s: error writing file (expected %d, got %d at ctr %d\n", 
				argv[0], nbytes, nwritten, ctr);
			break;
		}
		totalbytes += nbytes;
	}
	/* Set or clear the sticky bit */
	if (op == ENCRYPT) {
		/* Set the encrypt bit */
		if (chmod(filename, st.st_mode | S_ISVTX) < 0 ) {
			perror("chmod:");
		}
	} else if (op == DECRYPT) {
		/* Clear the encrypt bit */
		if (chmod(filename, st.st_mode & ~S_ISVTX) < 0) {
			perror("chmod");
		}
	}
	close(fd);
}