// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
//#include <core.c>
#include <sys/wait.h>
#include <sys/stat.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void
IO_redirect(command_t c){
	if(c->input != NULL){
		int fd = open(c->input, O_RDONLY);
		//Fail to open the file
		if(fd < 0){
			print_system_error();
		}
		//Fail to set standard input
		if(dup2(fd,STDIN_FILENO) < 0){
			print_system_error();
		}
		//Fail to close file descriptor
		if(close(fd) < 0){
			print_system_error();
		}
	}
	else if(c->output != NULL){
		int fd = open(c->output, O_CREATE | O_WRONLY);
		if(fd < 0){
			print_system_error();
		}
		if(dup2(fd,STDOUT_FILENO) <0){
			print_system_error();
		}
		if(close(fd) < 0){
			print_syste_error();
		}
	}
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

	argv = cmd->u.word;
	pid = fork();
	if(pid < 0){
		//could not fork child process
	}
	else if(pid == 0){
		//inside child process
		//Set IO
		IO_redirect(cmd);
		if(execvp(argv[0], argv) < 0){
			//Fail to execute simple command
			print_system_error();
		}

	}
	else{
		//inside parent process
		//wait for child process to finish
		if( waitpid(pid, &status,-1) == -1){
			print_system_error();
		}
	}
	cmd->status = WIFEXITED(status);
}

void execute_pipe_command(command_t cmd){

}

void execute_or_command(command_t cmd){
	command_t left = cmd->u.command[0];
	command_t right = cmd->u.command[1];
	execute_command(left,false);
	cmd->status = left->status;
	if(left->status != 0){
		execute_command(right,false);
		cmd->status = right->status;
	}
}

void execute_sequence_command(command_t cmd){

}

void execute_and_command(command_t cmd){
	command_t left = cmd->u.command[0];
	command_t right = cmd->u.command[1];
	//Use false for now, as we may change it later for part c
	execute_command(left,false);
	cmd->status = left->status;
	//execute rightside command if leftside exit normally
	if(left->status == 0){
		execute_command(right,false);
		cmd->status = right->status;
	}
}

void execute_subshell_command(command_t cmd){
	//TODO: Do IO here
	IO_redirect(cmd);
	command_t subshell_cmd = cmd->u.subshell_command;
	execute_command(subshell_command, time_travel);
	cmd->status = subshell_command->status;
}


