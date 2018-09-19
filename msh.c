/*
 Name: Olivier Ndikumana
 ID: 1001520973
*/


#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <dirent.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

#define MAX_NUM_PREVIOUS_PIDS 15 // Only 15 previous pids can be displayed

#define MAX_NUM_HISTORY 15 // Only 15 previous history files will be displayed


pid_t pid;

// creating array of commands to store history and an index to keep track of them
char history [MAX_NUM_HISTORY] [MAX_COMMAND_SIZE];
int currentHistoryIndex;

// creating array of PIDS and an index to keep track of them
int pids [MAX_NUM_PREVIOUS_PIDS];
int currentPidsIndex;



void printHistory() {
	// prints data in the histry array and does not return anything
	int i = 0;
	
	// makes index i start from last 15 commands but does not delete all other previous commands
	if (currentHistoryIndex > MAX_NUM_HISTORY) {
		i = currentHistoryIndex - MAX_NUM_HISTORY;
	}
	
	for (; i < currentHistoryIndex; i++ ) {
		printf("%d: %s",  i, history[i]);
	}
}

void printPids() {
	// prints data in the pids array and does not return anything
	int i = 0;
	
	// makes index i start from last 15 pids but does not delete all other previous pids
	if (currentPidsIndex > MAX_NUM_PREVIOUS_PIDS) {
		i = currentPidsIndex - MAX_NUM_PREVIOUS_PIDS;
	}

	for (;i < currentPidsIndex; i++ ) {
		printf("%d: %d\n", i, pids[i]);
	}
}

int executeCmd( char cmd [], char **argv )
{
	// check if user wants to close shell
	if ( (strcmp(cmd, "exit") == 0) || (strcmp(cmd, "quit") == 0) )
		return 1;
	
	// check if user wants view history
	if ( (strcmp(cmd, "history") == 0) ) {
		printHistory();
		return 0;
	}
	
	// check if user wants a list of previous pids
	if ( (strcmp(cmd, "listpids") == 0) || (strcmp(cmd, "showpids") == 0) ) {
		printPids();
		return 0;
	}
	
	// check if user wants to run interrupted proccess in background
	if ( strcmp("bg", cmd) == 0 ) {
		kill(pid, SIGCONT);
		return 0;
	}
	
	if ( (strcmp(cmd, "cd") == 0) && argv[1]) {
		chdir(argv[1]);
		return 0;
	}

	pid = fork();
	
	// Add pid to list of pids and increment current index TODO deal with the array being full and reallocating space
	pids[currentPidsIndex] = pid;
	currentPidsIndex++;
	

	if( pid == -1 )
	{
		// When fork() returns -1, an error happened.
		perror("fork failed: ");
		exit( EXIT_FAILURE );
	}
	else if ( pid == 0 )
	{
		// When fork() returns 0, we are in the child process.
		
			int error = execvp(cmd, argv);
			if (error) {
				printf("%s: Command not found\n", cmd);
				exit(0);
			}
	}
	else
	{
		// When fork() returns a positive number, we are in the parent
		// process and the return value is the PID of the newly created
		// child process.
		int status;

		// Force the parent process to wait until the child process
		// exits
		waitpid(pid, &status, 0 );
		fflush( NULL );
	}
	return 0;
}

static void handle_signal (int sig ) {
	int status;
	switch(sig){
		// suspends proccess. Only handles SIGSTP
        case SIGTSTP:
             waitpid(-1, &status, WNOHANG);
             break;
     }
}


int main() {

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
	  
	// check if the user is trying to access a command from history
	  if ( cmd_str && (cmd_str[0] == '!') ) {
		  char *cmdIndexToken = strtok(cmd_str, "!");
		  
		  // gets number from user input and tries to access it from the history array
		  int cmdIndex = atoi(cmdIndexToken);
		  if (cmdIndex) {
			  if ( (cmdIndex <= currentHistoryIndex) && (strlen(history[cmdIndex]) > 1) ) {
				  // replaces the cmd_str with data from stored history at appropriate index
				  strcpy(cmd_str, history[cmdIndex]);
			  }
			  else {
				  // if there is no command at that index
				  printf("Command does not exists in history\n");
				  continue;
			  }
		  }
		  else {
			  // if atoi returns 0
			  printf("Not a valid number\n");
			  continue;
		  }
	  }
	  
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS)) {

      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 ) {
        token[token_count] = NULL;
      }
        token_count++;
    }
	  
	  
	  // handling SIGTSTP && SIGINT
	  struct sigaction act;
	  memset (&act, '\0', sizeof(act));
	  act.sa_handler = &handle_signal;
	  if (sigaction(SIGTSTP , &act, NULL) < 0 || sigaction(SIGINT , &act, NULL) < 0) {
		  perror ("sigaction: ");
		  return 1;
	  }
	  
	  
	// checks if there is a command
	if (token[0]) {
		// adds command to history and increments history index
		strcpy(history[currentHistoryIndex], cmd_str);
		currentHistoryIndex++;
		
		// takes the command and the rest of argument to executeCmd function
		int result = executeCmd(token[0], token);
		
		// 1 is returned if command is quit or exit
		if (result) {
			return 0;
		}
	}



    free( working_root );

  }
  return 0;
}
