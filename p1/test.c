// Graham Greving
// CMPS 111 Operating Systems - Program 1
// The G(raham) Shell

#define _POSIX_SOURCE   /* for MAX_INPUT */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define PROMPT          "$: "
#define DELIMITERS      " \t" /* whitespace, tab */

// Function declarations
int makeargv(const char *s, const char *delimiters, char ***argvp);
int freemakeargv(char **argv);
int parseargv(int argc, char **argv);
int exec_command(char **cmdstring);
static void sig_chld(int);
int chp(char *newprompt);


char prompt[4];

// Global background and zombie flags
int bg;
int zombie;
int fdout;
int fdin;


int main(int argc, char **argv) {

	char input[MAX_INPUT];
	int numtokens;
	char **tokens;

	prompt[0] = '>';
	prompt[1] = '>';
	prompt[2] = ' ';
	prompt[3] = '\0';

	// Init the flags
	zombie = -1;
	bg = 0;

	// Print opening text
	printf("Welcome to the G(raham) Shell!\n");
	printf("To exit, type exit\n");

	// Loop indefinitely.
	while (1) {
		// Print the prompt!
		printf("%s", prompt);
		
		// Get the input line!
		// Move on if input is null or empty newline
		if (fgets(input, MAX_INPUT, stdin) == NULL) {
			// Fixes a bug where the signal handler interrupts the read system
			// call and forces an infinite prompt loop.
			// Thanks to Daniel Bittman and Justin Lardinois on Piazza :)
			clearerr(stdin);
			continue;
		}
		if (input[0] == '\n') continue;
		
		// Cut trailing newline
		int inputlen = strlen(input);
		if (input[inputlen - 1] == '\n') input[inputlen - 1] = 0;
		
		// Get the tokens
		numtokens = makeargv(input, DELIMITERS, &tokens);
		// Parse the command line!
		parseargv(numtokens, tokens);
		
		// Clean up
		freemakeargv(tokens);
	}
}

// Given an input string from the command line, parses
// for special characters, &, &&, >, <, acts accordingly,
// forks and executes the necessary processes.
int parseargv(int argc, char **argv) {
	//printf("debug: count: %d argv[0]: %s.\n", argc, argv[0]);

	int n = 0;
	int i = 0;
	fdin = -1;
	fdout = -1;
	char *cmdstring[argc];

	// Loop through the tokens, creating the command string
	for (i = 0; i < argc; i++) {
		if (strcmp (argv[i], "&&") == 0) {		// Compound execution
			if (bg == 1 || argc < 3) {
				printf("Sorry, bad syntax.\n");
				return (-1);
			}
			int rval = 0;
			printf("executing.\n");
			cmdstring[]
			rval = exec_command(cmdstring);
			if (rval == 0) {
				i++; // increment i, for proper args to argv
				parseargv(argc - i, &argv[i]);
				exit (1);
			} else {
				exit (-1);
			}
		} else {									// To be executed.
			cmdstring[n++] = argv[i];
		}
	}
	// Null terminate the command string
	cmdstring[n] = 0;
	// Execute the command!
	if (exec_command(cmdstring) < 0) {
		exit (-1);
	}
	return (1);
}

int exec_command(char **cmdstring) {
	printf("cmd: %s\n", cmdstring[0]);
	int status;
	// Fork and execute the command string
	int pid = fork();
	if (pid < 0) {				// Fork error
		perror("parseargv():fork()");
		return (-1);
	} else if (pid == 0) {		// Child, execute
		if (execvp(cmdstring[0], cmdstring) < 0) {
			perror("parseargv():execvp()");
			return(-1);
		} else {
			printf("executed ok\n");
			exit(1);
		}
	} else {					// Parent, wait
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			return (WEXITSTATUS(status));
		} else {
			return (-1);
		}
	}
}
