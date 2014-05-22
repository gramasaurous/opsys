#include <stdio.h>
#include <stdlib.h>

int main (void) {
	if (setkey(129,512) < 0) {
		printf("error\n");
	}
}
