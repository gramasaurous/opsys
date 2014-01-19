#define _POSIX_SOURCE   /* for MAX_INPUT */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>


#define PROMPT          "prompt: "
#define DELIMITERS      " \t" /* whitespace, tab */

int makeargv(const char *s, const char *delimiters, char ***argvp);
int freemakeargv(char **argv);
int parseargv(int argc, char **argv);

// Background flag
int bg;

int main(int argc, char **argv) {
	char inputbuffer[MAX_INPUT];
	int numtokens;
	char **tokens;

	while (1) {
		bg = 0;
		printf(PROMPT);
		/* we use fgets (gets may write past the buffer size) */
		if (fgets(inputbuffer, MAX_INPUT, stdin) == NULL)
		    continue;
		/* remove trailing newline (gets itself does this) */
		int inputlen = strlen(inputbuffer);
		if (strcmp(inputbuffer, "\n") == 0) continue;
		if (inputbuffer[inputlen - 1] == '\n')
		    inputbuffer[inputlen - 1] = 0;
		numtokens = makeargv(inputbuffer, DELIMITERS, &tokens);
		if (parseargv(numtokens, tokens) < 0) {
			printf("parseargv error\n");
			exit (-1);
		}

		freemakeargv(tokens);
	}

	return 0;
}

int parseargv(int argc, char **argv) {
	int i;
	int n = 0;
	// for opening
	int fd;
// Internal shell command, exit
	if (!strcmp(argv[0], "exit")) {
		printf("Exiting shell.\n");
		freemakeargv(argv);
		exit(0);
	}
//	char **cmdstring;
	char *cmdstring[argc];

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "&")) {
			printf("set bg flag. don't wait, catch SIGCHLD\n");
			bg = 1;
		} else if (!strcmp(argv[i], "&&")) {
			printf("exec cmdstring, if returns OK, parse argv[i+1]\n");	
		} else if (!strcmp(argv[i], ">")) {
			printf("open %s\n", argv[++i]);
			close (1);
			fd = open(argv[i], O_CREAT | O_WRONLY, 0400 | 0200);
			if (fd <= 0) {
				perror("parseargv(): open()");
				return (-1);
			}
		} else if (!strcmp(argv[i], "<")) {
			printf("close stdin, open %s\n", argv[++i]);
		} else {
			printf("exec %s\n", argv[i]);
			cmdstring[n++] = argv[i];
		}
	}
	cmdstring[n] = NULL;

	// Fork!	
	int pid = fork();
	if (pid < 0) {						// Fork Error
		perror("parseargv(): fork():");
		return (-1);
	} else if (pid == 0 ) { 				// Child, exec
		if (execvp(cmdstring[0], cmdstring) == -1) {
			perror("parseargv():execvp():");
			return (-1);
		}
		close (fd);
	} else { 							// Parent, wait
		int status;
		close (fd);
		waitpid(pid, &status, 0);
		return (1);
	}
	return (0);
}

