#define _POSIX_SOURCE   /* for MAX_INPUT */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <glob.h>
#include <fcntl.h>

/* 
	Struct holding variables for each possible special case,
	and typedef for readability
*/
struct Vars {
	int rin;
	int rout;
	int bg;
	int pipe;
};
typedef struct Vars var_s;

/*
	Helper Function Prototypes, defined at bottom
*/
	#define DELIMITERS      " \t" /* whitespace, tab */

int makeargv(const char *s, const char *delimiters, char ***argvp);
int freemakeargv(char **argv);
int parseargv(int argc, char **argv);
	// Prompt
int prompt(void);
	// Command Parser
char **getCommand(char **args, var_s *vars);
	// Command Executioner
int execCommand(char **cmd, char **args, var_s *vars);
	// Struct init
int varsInit(var_s *vars);

int printDebug(char **args);

int main (void) {
	char **args;
	char **cmd;
	int i;
	var_s variables;

	char inputbuffer[MAX_INPUT];
	int numtokens;
	char **tokens;

//	prompt();
	while (1) {
			// Display the prompt
		prompt();
			// Initialize the Variables
		varsInit(&variables);
			// Get the arguments
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
		if (variables.rin != -1) {
			if (execRedin(cmd, args[variables.rin+1], &variables) == -1) {
				break;
			}
		} else if (variables.rout != -1) {
			if (execRedout(cmd, args[variables.rout+1], &variables) == -1) {
				break;
			}
		} else {
			if (execCommand(cmd, args, &variables) == -1 ) {
				break;
			}
		}
			// Clear the command
		for (i = 0; cmd[i] != NULL; i++) {
			free(cmd[i]);
			cmd[i] = NULL;
		}
		cmd = NULL;
		args = NULL;
	}
}

char **getCommand(char **args, var_s *vars) {
	char **cmd;
	int i = 0;
	int n = 0;
		// Handle Internal Shell Command exit
	if (strcmp(args[0], "exit") == 0) {
		printf("Exiting!\n");
		exit(0);
	}
		// Handle Special Character Tokens
	for (i = 0; args[i] != NULL; i++) {
		if (strcmp(args[i], "|") == 0) {			// Pipe
			vars->pipe = i;
		} else if (strcmp(args[i], ">") == 0) {		// Redout
			vars->rout = i;
			break;
		} else if (strcmp(args[i], "<") == 0) {		// Redin
			vars->rin = i;
			break;
		} else if (strcmp(args[i], "&") == 0) {		// background
			vars->bg = 1;
		} else {									// cmd, opt or file
			cmd[n++] = strdup(args[i]);
		}
	}
	cmd[n] = NULL;
	return cmd;
}

int execCommand(char **cmd, char **args, var_s *vars) {
	//printf("Executing a command\n");
	int pid = fork();
	if (pid < 0) {
		printf("Error: fork()\n");
		return -1;
	} else if (pid == 0) {
		if (vars->bg != 0) {
			//printf("Background: %d\n", pid);
		}
		if (execvp(cmd[0], cmd) == -1) {
			printf("Error: couldn't execute %s.\n", cmd[0]);
			return -1;
		}
	} else {
		int status;
		if (vars->bg == 0) {
			waitpid(pid, &status, 0);
		} else {
			printf("Background %d\n",pid);
		}
		// Create a function to handle signals and errors
	}
	return 1;
}

// Execute a command with a file output redirect
int execRedout(char **cmd, char *file, var_s *vars) {
	int fd = open(file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error: %s cannot be opened/created for writing.\n", file);
		return -1;
	}
	int pid = fork();
	if (pid < 0) {
		printf("Error: fork() failed\n");
		return -1;
	} else if (pid == 0) {
		dup2(fd, 1);
		close(fd);
		if (execvp(cmd[0], cmd) == -1) {
			printf("Error: couldn't execute %s.\n", cmd[0]);
			return -1;
		}
		clearerr(stdout);
	} else {
		int status;
		close (fd);
		if (vars->bg == 0) {
			waitpid(pid, &status, 0);
		} else {
			printf("Background: %d\n",pid);
		}
	}
	return 1;
}

// Execute a command with a file input redirect
int execRedin(char **cmd, char *file, var_s *vars) {
	int fd = open(file, O_RDONLY);
	if (fd == -1) {
		printf("Error: %s cannot be opened for reading.\n", file);
		return -1;
	}
	int pid = fork();
	if (pid < 0) {
		printf("Error: fork() failed\n");
		return -1;
	} else if (pid == 0) {
		dup2(fd, 0);
		close(fd);
		if (execvp(cmd[0], cmd) == -1) {
			printf("Error: couldn't execute %s.\n", cmd[0]);
			return -1;
		}
	} else {
		close (fd);
		int status;
		if (vars->bg == 0) {
			waitpid(pid, &status, 0);
		} else {
			printf("Background: %d\n",pid);
		}
	}
	return 1;
}

int prompt(void) {
	char *buf = malloc(sizeof(char)*1024);
	getcwd(buf, 1024);
	printf("username:%s $ ", buf);
	free(buf);
	return 1;
}

/*
	Initialize the variables. All except for background
	are init'd to -1 so they can be set to a position
	on the argument array.
*/
int varsInit(var_s *vars) {
	vars->bg = 0;
	vars->rin = -1;
	vars->rout = -1;
	vars->pipe = -1;
	return 1;
}

int printDebug(char **args) {
	int i = 0;
	for (i = 0; args[i] != NULL; i++) {
		printf("arg %d: %s\n", i, args[i]);
	}
}
