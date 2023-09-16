#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

#define MAXWORDS 100	// Maximum space separated strings in one command
typedef enum{DEFAULT, PARA, SEQ, REDIRECT} parse_status_code;	// Different execution modes: 
/*DEFAULT - single command, PARA - parallel execution, SEQ - sequential execution, REDIRECT - redirection */

// This function will parse the input string into space-separated strings (including arguments)
parse_status_code parseInput(char *words[MAXWORDS], char *cmd)
{
	int i = 0;
	parse_status_code retval = DEFAULT;
	char* word = NULL;

	// Split the command into space-separated strings and storing in a 2D-array
	word = strsep(&cmd, " ");
	while(word != NULL) {
		if(word[0] != ' ' && word[0] != '\0')
			words[i++] = word;
		word = strsep(&cmd, " ");
	}

	words[i] = NULL;	//Terminate the end of 2D array to denote the end of array

	for(i=0; words[i] != NULL; i++) {
		if(strcmp(words[i], "&&") == 0) {	// Parallel execution case
			retval = PARA;
		}
		else if(strcmp(words[i], "##") == 0) {	// Sequential execution case
			retval = SEQ;
		}
		else if(strcmp(words[i], ">") == 0) {	// File redirection case
			retval = REDIRECT;
		}
		// printf("%s\n", words[i]);	// Debugging
	}

	return retval;
}

// This function will fork a new process to execute a command
void executeCommand(char *args[MAXWORDS])
{
	int errStatus;
	if(strcmp(args[0], "cd") == 0) {	// Special handling case for changing directory because it involves with the parent's environment
		if(args[1] == NULL) {
			chdir(getenv("HOME"));
		}
		else {
			errStatus = chdir(args[1]);
			if(errStatus == -1)
				printf("bash: cd: %s: No such file or directory\n", args[1]);
		}
	}
	else {
		int cpid = fork();
		if (cpid < 0) {	// If fork fails			
			exit(1);
		}
		else if (cpid == 0) {	// child
			signal(SIGINT, SIG_DFL);	// Enable SIGINT signal in child
			signal(SIGTSTP, SIG_DFL);	// Enable SIGSTSTP signal in child

			errStatus = execvp(args[0], args);
			if(errStatus == -1) {	// If exec fails
				printf("Shell: Incorrect command\n");
				exit(1);
			}
		}
		else {	// parent
			int status, c_wait;
			c_wait = waitpid(WAIT_ANY, &status, WUNTRACED);	// parent waits for child also tracking for SIGTSTP signal
			if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTSTP) {	// If child process has stopped due to SIGTSTP signal
				kill(c_wait, SIGKILL);	// Kill the stopped process
			}
		}
	}
}

// This function will run multiple commands in parallel
void executeParallelCommands(char *words[MAXWORDS]) {
	int i, j, errStatus, cpid;
	char *args[MAXWORDS];

	int done = 0;
	i = 0;
	j = 0;
	while(!done) {
		if(words[i] == NULL || strcmp(words[i], "&&") == 0) {		// If a delimitter like && or NULL occurs, execute the command
			if(words[i] == NULL) {
				done = 1;
			}
			args[j] = NULL;
			i++;
			j=0;
			
			if(strcmp(args[0], "cd") == 0) {	// Special handling case for changing directory because it involves with the parent's environment
				if(args[1] == NULL) {
					chdir(getenv("HOME"));
				}
				else {
					errStatus = chdir(args[1]);
					if(errStatus == -1)
						printf("bash: cd: %s: No such file or directory\n", args[1]);
				}
			}
			else {
				int cpid = fork();
				if (cpid < 0) {	// If fork fails
					exit(1);
				}
				else if (cpid == 0) {	// child
					signal(SIGINT, SIG_DFL);	// Enable SIGINT signal in child
					signal(SIGTSTP, SIG_DFL);	// Enable SIGSTSTP signal in child
					errStatus = execvp(args[0], args);
					if(errStatus == -1) {	// If exec fails
						printf("Shell: Incorrect command\n");
						exit(1);
					}
				}
			}
		}
		else {	// Gather all the words till a full command is formed(till a delimitter like NULL or && occurs)
			args[j++] = words[i++];
		}
	}
	// parent
	int status, c_wait;
	while ((c_wait = waitpid(WAIT_ANY, &status, WUNTRACED)) > 0) {	// parent waits till all child processes either exit/terminate/stop
		if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTSTP) {	// If child process has stopped due to SIGTSTP signal
			kill(c_wait, SIGKILL);	// Kill the stopped process
		}
	}
}

// This function will run multiple commands in sequence
void executeSequentialCommands(char *words[MAXWORDS])
{
	int i, j, errStatus, done = 0;
	char *args[MAXWORDS];
	
	i = 0; j = 0;
	while(!done) {
		if(words[i] == NULL || strcmp(words[i], "##") == 0) {		// If a delimitter like ## or NULL occurs, execute the command
			if(words[i] == NULL)
				done = 1;

			args[j] = NULL;
			j = 0;
			i++;
			
			if(strcmp(args[0], "cd") == 0) {	// Special handling case for changing directory because it involves with the parent's environment
				if(args[1] == NULL) {
					chdir(getenv("HOME"));
				}
				else {
					errStatus = chdir(args[1]);
					if(errStatus == -1)
						printf("bash: cd: %s: No such file or directory\n", args[1]);
				}
			}
			else {
				int cpid = fork();
				if (cpid < 0) {	// If fork fails
					exit(1);
				}
				else if (cpid == 0) {	// child
					signal(SIGINT, SIG_DFL);	// Enable SIGINT signal in child
					signal(SIGTSTP, SIG_DFL);	// Enable SIGSTSTP signal in child

					errStatus = execvp(args[0], args);
					if(errStatus == -1) {	// If exec fails
						printf("Shell: Incorrect command\n");
						exit(1);
					}
				}
				else {	// parent
					int status, c_wait;
					c_wait = waitpid(WAIT_ANY, &status, WUNTRACED);	// parent waits for child process till it either exits/terminates/stops
					if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTSTP) {	// If child process has stopped due to SIGTSTP signal
						kill(c_wait, SIGKILL);	// Kill the stopped process
						return;	// Exit the loop to prevent execution of remaining commands
					}
					else if(WIFSIGNALED(status) && WTERMSIG(status) == SIGINT) {		// If child process has stopped due to SIGINT signal
						return;	// Exit the loop to prevent execution of remaining commands
					}
				}
			}
		}
		else {	// Gather all the words till a full command is formed(till a delimitter like NULL or && occurs)
			args[j++] = words[i++];
		}
		
	}
}

// This function will run a single command with output redirected to an output file specificed by user
void executeCommandRedirection(char *args[MAXWORDS])
{
	int i, errStatus, cpid;

	cpid = fork();
	if (cpid < 0) {	// If fork fails
		exit(1);
	}
	else if (cpid == 0) {	// child
		signal(SIGINT, SIG_DFL);	// Enable SIGINT signal in child
		signal(SIGTSTP, SIG_DFL);	// Enable SIGSTSTP signal in child

		// Collect the words required to form a command(till a delimmiter like '>' occurs)
		i = 0;
		while(strcmp(args[i], ">") != 0) {
			i++;
		}
		args[i++] = NULL;

		close(STDOUT_FILENO);	// Close STDOUT file descriptor
		open(args[i], O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);	// Open the file whose name has been passed as an argument

		if(strcmp(args[0], "cd") != 0) {
			errStatus = execvp(args[0], args);
			if(errStatus == -1) {	// If exec fails
				printf("Shell: Incorrect command\n");
				exit(1);
			}
		}
		else {	// Since cd command doesn't give any output, simply exit
			exit(0);
		}
	}
	else {	// parent
		int c_wait, status;
		c_wait = waitpid(WAIT_ANY, &status, WUNTRACED);	// parent waits for child process till it either exits/terminates/stops
		if(WIFSTOPPED(status)) {	// If child process has stopped due to SIGTSTP signal
			kill(c_wait, SIGKILL);	// Kill the stopped process
		}

		if(strcmp(args[0], "cd") == 0) {	// Special handling case for changing directory because it involves with the parent's environment
			if(args[1] == NULL) {
				chdir(getenv("HOME"));
			}
			else {
				errStatus = chdir(args[1]);
				if(errStatus == -1)
					printf("bash: cd: %s: No such file or directory\n", args[1]);
			}
		}
	}
}

int main()
{
	// Ignore basic signals in parent
	signal(SIGINT, SIG_IGN);	// Ignore SIGINT (The signal which gets called when Ctrl+C is pressed in keyboard)
	signal(SIGTSTP, SIG_IGN);	// Ignore SIGTSTP (The signal which gets called when Ctrl+Z is pressed in keyboard)
	signal(SIGCHLD, SIG_IGN);	// Ignore SIGCHLD (This signal is ignored to cause zombie processes to be reaped automatically. A child process state can be converted to a zombie state after terminating stopped child process using kill command)

	// Initial declarations
	char *currentWorkingDirectory, *cmd = NULL;
	size_t cmdLen, bufferLen;
	char *words[MAXWORDS];
	parse_status_code sc;

	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		currentWorkingDirectory = getcwd(NULL, 0);
		printf("%s$", currentWorkingDirectory);
		
		// accept input with 'getline()'
		cmdLen = getline(&cmd, &bufferLen, stdin);
		cmd[cmdLen-1] = '\0';

		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		sc = parseInput(words, cmd);
		
		if(!words[0]) {		// When new line/whitespaces are given, ignore this iteration of while loop
			continue;
		}

		if(strcmp(words[0], "exit") == 0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		
		if(sc == PARA)
			executeParallelCommands(words);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(sc == SEQ)
			executeSequentialCommands(words);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(sc == REDIRECT)
			executeCommandRedirection(words);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else
			executeCommand(words);		// This function is invoked when user wants to run a single commands

		free(currentWorkingDirectory);
		free(cmd);
		currentWorkingDirectory = cmd = NULL;
	}
	
	return 0;
}
