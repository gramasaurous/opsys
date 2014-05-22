/*
 * This code encrypts input data using the Rijndael (AES) cipher.  The
 * key length is hard-coded to 128 key bits; this number may be changed
 * by redefining a constant near the start of the file.
 *
 * This program uses CTR mode encryption.
 *
 * Usage: encrypt <key1> <key2> <file name>
 *
 * Author: Ethan L. Miller (elm@cs.ucsc.edu)
 * Based on code from Philip J. Erdelsky (pje@acm.org)
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "rijndael.h"

static char rcsid[] = "$Id: encrypt.c,v 1.2 2003/04/15 01:05:36 elm Exp elm $";

#define KEYBITS 128

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


/***********************************************************************
 *
 * getpassword
 *
 * Get the key from the password.  The key is specified as a string of
 * hex digits, two per key byte.  password points at the character
 * currently being added to the key.  If it's '\0', the key is done.
 *
 ***********************************************************************
 */
void
getpassword (const char *password, unsigned char *key, int keylen)
{
	int   i;

	for (i = 0; i < keylen; i++) {
		if (*password == '\0') {
			key[i] = 0;
		} else {
			/* Add the first of two digits to the current key value */
			key[i] = hexvalue (*(password++)) << 4;
			/* If there's a second digit at this position, add it */
			if (*password != '\0') {
				key[i] |= hexvalue (*(password++));
			}
		}
	}
}

int main(int argc, char **argv)
{
	unsigned long rk[RKLENGTH(KEYBITS)];    /* round key */
	unsigned char key[KEYLENGTH(KEYBITS)];  /* cipher key */
	char  buf[100];
	int i, nbytes, nwritten , ctr;
	int totalbytes;
	int k0, k1;
	int fileId = 0x1234;
	int nrounds;          /* # of Rijndael rounds */
	char *password;       /* supplied (ASCII) password */
	int fd;
	char *filename;
	unsigned char filedata[16];
	unsigned char ciphertext[16];
	unsigned char ctrvalue[16];


#if 0
	if (argc < 3)
	{
	fprintf (stderr, "Usage: %s <key> <file>\n", argv[0]);
	return 1;
	}
	/*
	 * Get the key from the password.  The key is specified as a string of
	 * hex digits, two per key byte.  password points at the character
	 * currently being added to the key.  If it's '\0', the key is done.
	 */
	getpassword (argv[1], key, sizeof (key));
	filename = argv[2];
#else
	if (argc < 4)
	{
	fprintf (stderr, "Usage: %s <key1> <key2> <file>\n", argv[0]);
	return 1;
	}
	bzero (key, sizeof (key));
	k0 = strtol (argv[1], NULL, 0);
	k1 = strtol (argv[2], NULL, 0);
	bcopy (&k0, &(key[0]), sizeof (k0));
	bcopy (&k1, &(key[sizeof(k0)]), sizeof (k1));
	filename = argv[3];
#endif

	/* Print the key, just in case */
	for (i = 0; i < sizeof (key); i++) {
		sprintf (buf+2*i, "%02x", key[sizeof(key)-i-1]);
	}
	fprintf (stderr, "KEY: %s\n", buf);

	/*
	 * Initialize the Rijndael algorithm.  The round key is initialized by this
	 * call from the values passed in key and KEYBITS.
	 */
	nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);

	/*
	 * Open the file.
	 */
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
	fprintf(stderr, "Error opening file %s\n", argv[2]);
	return 1;
	}

	/* fileID goes into bytes 8-11 of the ctrvalue */
	bcopy (&fileId, &(ctrvalue[8]), sizeof (fileId));
	for (i = 0; i < sizeof(ctrvalue); i++) printf("%d\n", ctrvalue[i]);


	/* This loop reads 16 bytes from the file, XORs it with the encrypted
	 CTR value, and then writes it back to the file at the same position.
	 Note that CTR encryption is nice because the same algorithm does
	 encryption and decryption.  In other words, if you run this program
	 twice, it will first encrypt and then decrypt the file.
	*/
	for (ctr = 0, totalbytes = 0; /* loop forever */; ctr++)
	{
		/* Read 16 bytes (128 bits, the blocksize) from the file */
		nbytes = read (fd, filedata, sizeof (filedata));
		if (nbytes <= 0) {
			break;
		}
		if (lseek (fd, totalbytes, SEEK_SET) < 0)
		{
			perror ("Unable to seek back over buffer");
			exit (-1);
		}

		/* Set up the CTR value to be encrypted */
		bcopy (&ctr, &(ctrvalue[0]), sizeof (ctr));

		/* Call the encryption routine to encrypt the CTR value */
		rijndaelEncrypt(rk, nrounds, ctrvalue, ciphertext);

		/* XOR the result into the file data */
		for (i = 0; i < nbytes; i++) {
			filedata[i] ^= ciphertext[i];
		}

		/* Write the result back to the file */
		nwritten = write(fd, filedata, nbytes);
		if (nwritten != nbytes)
		{
			fprintf (stderr,
				 "%s: error writing the file (expected %d, got %d at ctr %d\n)",
				 argv[0], nbytes, nwritten, ctr);
			break;
		}

		/* Increment the total bytes written */
		totalbytes += nbytes;
	}
	close (fd);
}
