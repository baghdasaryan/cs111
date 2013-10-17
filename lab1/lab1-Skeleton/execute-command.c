// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <core.c>
#include <sys/wait.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  enum command_type cur_cmd_type = c->type;
  switch (cur_cmd_type) {
  	case AND_COMMAND: 

  	case SEQUENCE_COMMAND: 

  	case OR_COMMAND:

  	case PIPE_COMMAND:

  	case SIMPLE_COMMAND:

  	case SUBSHELL_COMMAND:

  	default: //Some error handling code

  }
  error (1, 0, "command execution not yet implemented");
}

//fork a child process to execute simple command
void execute_simple_command(command_t cmd, bool time_travel){
	pid_t pid;
	int status;
	char ** argv;
	char * filename;

	pid = fork();
	if(pid < 0){
		//could not fork child process
	}
	else if(pid == 0){
		//inside child process
		//
		if(execvp(filename, argv) < 0){
			//Fail to execute simple command
			print_system_error();
		}

	}
	else{
		//TODO: Do IO here
		//inside parent process
		//wait for child process to finish
		if( waitpid(pid, &status,-1) == -1){
			print_system_error();
		}
	}
	cmd->status = WIFEXITED(status);
}

void execute_pipe_command(command_t cmd, bool time_travel){

}

void execute_or_command(command_t cmd, bool time_travel){

}

void execute_sequence_command(command_t cmd, bool time_travel){

}

void execute_and_command(command_t cmd, bool time_travel){

}

void execute_subshell_command(command_t cmd, bool time_travel){
	//TODO: Do IO here
	command_t subshell_cmd = cmd->u.subshell_command;
	execute_command(subshell_command, time_travel);
	cmd->status = subshell_command->status;
}
