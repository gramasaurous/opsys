#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv) {
	unsigned int k0;
	unsigned int k1;
	k0 = strtol(argv[1], NULL, 0);
	k1 = strtol(argv[2], NULL, 0);

	if (setkey(k0,k1) < 0) {
		printf("error\n");
	}
}
