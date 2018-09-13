// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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


int executeCmd( char cmd [], char **argv )
{
	if ( (strcmp(cmd, "exit") == 0) || (strcmp(cmd, "quit") == 0) )
		return 1;

	pid_t pid = fork();

	if( pid == -1 )
	{
		// When fork() returns -1, an error happened.
		perror("fork failed: ");
		exit( EXIT_FAILURE );
	}
	else if ( pid == 0 )
	{
		// When fork() returns 0, we are in the child process.

		if ( (strcmp(cmd, "cd") == 0) && argv[1]) {
			chdir(argv[1]);
		}
		else {
			int error = execvp(cmd, argv);
			if (error) {
				printf("%s: Command not found\n", cmd);
			}
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
//		printf("Hello from the parent process\n");
		fflush( NULL );
	}
	return 0;
}

static void handle_signal (int sig ) {
  printf ("Caught signal %d\n", sig );
	// switch(sig){
  //       case SIGCHLD:
  //            // note that the last argument is important for the wait to work
  //            waitpid(-1, &status, WNOHANG);
  //            break;
  //    }
}


int main() {

	// handling SIGTSTP
	struct sigaction act;
	memset (&act, '\0', sizeof(act));
	act.sa_handler = &handle_signal;
	if (sigaction(SIGTSTP , &act, NULL) < 0) {
		perror ("sigaction: ");
		return 1;
	}

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

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

//    int token_index  = 0;
//    for( token_index = 0; token_index < token_count; token_index ++ )
//    {
//      printf("token[%d] = %s\n", token_index, token[token_index] );
//    }

		// checks if there is a command
	  if (token[0]) {
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
