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
		// Clear the background flag 
		bg = 0;
		// Check for finished zombies.
		if (zombie > 0) {
			printf("Process %d terminated.\n", zombie);
			zombie = -1;
		}

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

	// Check for 'exit'
	if (strcmp(argv[0], "exit") == 0) {
		freemakeargv(argv);
		printf("Exiting!\n");
		exit (1);
	} else if (strcmp(argv[0], "chp") == 0) {
		chp(argv[1]);
		return (-1);
	}

	// Loop through the tokens, creating the command string
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i],"&") == 0) {					// Background process
			if (argc == 1) {
				printf("Sorry, bad syntax.\n");
				return (-1);
			}
			bg = 1;
		}  else if (strcmp (argv[i], "&&") == 0) {		// Compound execution
			if (bg == 1 || argc < 3) {
				printf("Sorry, bad syntax.\n");
				return (-1);
			}
			// null terminate the cmdstring
			cmdstring[n] = 0;
			if (exec_command(cmdstring) == 0) {
				i++; // increment i, for proper args to argv
				parseargv(argc - i, &argv[i]);
				return (1);
			} else {
				exit (-1);
			}
		} else if (strcmp(argv[i], ">") == 0) {			// Output redirection
			// Open file specified right after the special character
			if (argc == 1) {
				printf("Sorry, bad syntax\n");
				return (-1);
			}
			fdout = open(argv[++i], O_CREAT | O_WRONLY, 0400 | 0200);
			if (fdout < 0) {
				//perror("parseargv():open()");
				printf("Error: file cannot be opened for writing.\n");
				return (-1);
			}
		} else if (strcmp(argv[i], "<") == 0) { 	// Input redirection
			// Open file specified right after the special character
			if (argc == 1) {
				printf("Sorry, bad syntax.\n");
				return (-1);
			}
			fdin = open(argv[++i], O_RDONLY);
			if (fdin < 0) {
				printf("Error: file cannot be opened for reading.\n");
				return (-1);
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

	// Close files in case they didn't get closed
	close (fdin);
	close (fdout);
	return (1);

}

int exec_command(char **cmdstring) {
	if (cmdstring == NULL) {
		printf("bad ptr\n");
		return (-1);
	}
	// Fork and execute the command string
	int pid = fork();
	if (pid < 0) {				// Fork error
		perror("parseargv():fork()");
		return (-1);
	} else if (pid == 0) {		// Child, execute
		// Handle I/O redirection
		if (fdout >= 0) {
			dup2(fdout, 1);
			close(fdout);
		} if (fdin >= 0) {
			dup2(fdin, 0);
			close(fdin);
		}
		// Execute the command string.
		if (execvp(cmdstring[0], cmdstring) < 0) {
			perror("parseargv():execvp()");
			return(-1);
		} else {
			exit (1);
		}
	} else {					// Parent, wait
		int status = 0;
		// Close appropraite I/O redirect fidles
		if (fdout >= 0) {
			close(fdout);
		} if (fdin >= 0) {
			close(fdin);
		}
		// Process should be backgrounded.
		// Set up the SIGCHLD signal handler and print
		// the backgrounded process.
		if (bg == 1) {
			if (signal(SIGCHLD, sig_chld) == SIG_ERR) {
				perror("parseargv():signal()");
				return (-1);
			}
			printf("background: %d\n", pid);
			return (1);
		}  else {
			// Ignore SIGCHLD
			if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
				perror("parseargv():signal()");
				return (-1);
			}
			// Wait normally.
			waitpid(pid, &status, 0);
			if (WIFEXITED(status)) {
				return (WEXITSTATUS(status));
			} else {
				return (-1);
			}
		}
	}
}

// Catch SIGCHLD
// Wait for the zombie, and set global zombie
// variable to the zombies pid
static void sig_chld(int signo) {
	int status;
	if ((zombie = wait(&status)) < 0) {
		perror("sig_chld():wait():");
	}
}


// Change the prompt character.
int chp(char *newprompt) {
	if (newprompt == NULL) {
		printf("Didn't get that. You have to enter two characters to change the prompt.\n");
		return (-1);
	} else if (strlen(newprompt) != 2) {
		printf("Sorry. That prompt is the wrong size. Has to be two characters.\n");
		return (-1);
	} else {
		prompt[0] = newprompt[0];
		prompt[1] = newprompt[1];
		printf("Changing Prompt!\n");
		return 1;
	}
}