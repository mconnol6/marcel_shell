#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

int get_input(char** t);
void execute(char** argv, int v);
void wait_shell(int pid);
void send_signal(int pid, int command);
int check_if_int(char t[]);
void print_error();
void adventure();

enum {
	NOTHING,
	EXIT,
	START,
	WAIT,
	RUN,
	KILL,
	STOP,
	CONTINUE
};

int main() {

	//introduction to shell
	printf("\nMy name is Marcel and I'm partially a shell as you can see on my body.\n");
	//sleep(1);
	printf("\nSo I like that about myself.\n");
	//sleep(1);
	printf("\nAnd I like myself.\n");
	//sleep(1);
	printf("\nAnd I have other great qualities as well.\n");
	//sleep(1);
	printf("\nLike executing unix commands.\n");
	//sleep(1);
	printf("\n");

	int command; //use for return from get_input

	do {
		char * tokens[100]; //use to split input into tokens
		
		//set command to return value of get_input
		command = get_input(tokens);

		//switch case to determine how to proceed
		switch (command) {

			//if EXIT or NOTHING, break out of switch
			case EXIT:
			case NOTHING:
				break;

			//if RUN or START, execute command
			case RUN:
			case START:
				execute(tokens, command);
				break;
			//if WAIT
			case WAIT:
				wait_shell(-1);
				break;
			case KILL:
			case CONTINUE:
			case STOP:
				send_signal(atoi(tokens[0]), command);
				break;
			default:
				printf("Not defined behavior yet.\n");
		}

	} while (command != EXIT); 
}

//asks user for input and decides what the user wants to do
//returns an enum value to indicate how the shell should proceed
int get_input(char ** t) {

	char input[4096]; //buffer for input

	//ask for new input
	printf("marcel> ");
	fflush(stdout);
	fgets(input, 4096, stdin);

	//copy of input in case command is unknown
	char copy[4096];
	strncpy(copy, input, 4096);

	if (strcmp(input, "What do you do for adventure?\n") == 0) {
		adventure();
		return NOTHING;
	}
	
	char *first, *temp;

	//get first input
	first = strtok(input, " \t\n");

	//check if a command was entered; if not, return NOTHING
	if (first == NULL) {
		return NOTHING;
	}

	//set up variables to get tokens
	int i=0;

	//get tokens
	do {
		temp = strtok(NULL, " \t\n");
		t[i] = temp;
		i++;
	} while (temp != NULL);

	//if user wants to exit return EXIT
	if ((strcmp(first, "exit") == 0) && (t[0] == NULL)) {
		return EXIT;
	}

	//run
	if (strcmp(first, "run") == 0) {
		return RUN;
	}

	//start
	if (strcmp(first, "start") == 0) {
		return START;
	}

	//wait
	if ((strcmp(first, "wait") == 0) && (t[0] == NULL)) {
		return WAIT;
	}

	//kill
	//make sure third argument is NULL
	if (strcmp(first, "kill") == 0 && check_if_int(t[0]) && t[1] == NULL) {
		return KILL;
	}

	if (strcmp(first, "continue") == 0 && check_if_int(t[0]) && t[1] == NULL) {
		return CONTINUE;
	}

	if (strcmp(first, "stop") == 0 && check_if_int(t[0]) && t[1] == NULL) {
		return STOP;
	}

	printf("marcel: unknown command: %s", copy);

	return NOTHING;
}

//execute command in argv
//v is either START or RUN
void execute(char **argv, int v) {
	pid_t pid;
	int status;

	//fork parent process
	if ((pid = fork()) < 0) {
		printf("marcel: ");
		print_error();
		exit(1);
	}

	//if pid is great than 0
	//if v == START, print that child process has started 
	//if v == RUN, wait
	if (pid > 0) {
		if (v == START) {
			printf("marcel: process %i started\n", pid);
		} else if ( v == RUN) {
			//waitpid(pid, &status, 0);
			wait_shell(pid);
		}
	}

	//if pid is 0, execute command in argv
	if (pid == 0) {
		//execute child process. If pid is < 0, print error message
		if (execvp(*argv, argv) < 0) {
			printf("marcel: ");
			print_error();
		}
	}
}

//waits for a process to finish
//pid is negative if parent is waiting for any child
//if waiting for a specific child, then input pid
void wait_shell(int pid) {
	int status, return_value;

	//call wait
	if (pid < 0) {
		return_value = wait(&status);
	} else {
		return_value = waitpid(pid, &status, 0);
	}

	//first check return value
	// if there are no children to wait for
	if (return_value < 0 && pid < 0) {
		printf("marcel: no processes left\n");
	} else if (return_value < 0 && pid > 0) {
	// if there was an error waiting for a specific pid
		print_error();
	}
	
	//now check status
	else if (status == 0) {
	//if return was successful
		printf("marcel: process %i exited normally with status %i\n", return_value, status);
	} else if (WIFSIGNALED(status)) {
		//else check for errors
		printf("marcel: process %i exited abnormally with signal %i: %s\n", return_value, status, strsignal(WTERMSIG(status)));
	}
}

//send a signal to a process: kill, stop, or continue
void send_signal(int pid, int command) {

	//try to send correct signal
	//if successful, prints message
	//return after message
	//default just returns
	switch (command) {
		case KILL:
			if(kill(pid, SIGKILL) == 0) {
				printf("marcel: process %i killed\n", pid);
				return;
			}
			break;
		case CONTINUE:
			if(kill(pid, SIGCONT) == 0) {
				printf("marcel: process %i continued\n", pid);
				return;
			}
			break;
		case STOP:
			if(kill(pid, SIGSTOP) == 0) {
				printf("marcel: process %i stopped\n", pid);
				return;
			}
			break;
		default:
			return;
			break;
	}

	//else there is an error
	printf("marcel: ");
	print_error();
}

//use to make sure that a string can be converted to an integer
int check_if_int(char t[]) {
	int i=0;
	while (t[i] != '\0') {
		if (!(isdigit(t[i]))) {
			return 0;
		}
		i++; //increment i
	}
	return 1;
}

//function for printing error messages
void print_error() {
	printf("%s\n", strerror(errno));
}

void adventure() {
	printf("\nHang glide on a dorito.\n\n");
}

