// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <fcntl.h>


int
command_status (command_t c)
{
  return c->status;
}

//print system call error and exit
void
print_system_error(){
	fprintf(stderr, "%s\n", strerror(errno));
	exit(1);
}

void
IO_redirect(command_t c){
  if (c->input != NULL) {
    int fd = open(c->input, O_RDONLY);
    //Fail to open the file
    if (fd < 0) {
      print_system_error();
    }
    //Fail to set standard input
    if (dup2(fd, STDIN_FILENO) < 0) {
    	print_system_error();
    }
    //Fail to close file descriptor
    if (close(fd) < 0) {
      print_system_error();
    }
  }
  else if (c->output != NULL) {
    int fd = open(c->output, O_CREAT | O_WRONLY);
    if (fd < 0) {
      print_system_error();
    }
    if (dup2(fd, STDOUT_FILENO) <0) {
      print_system_error();
    }
    if (close(fd) < 0) {
      print_system_error();
    }
  }
}

//fork a child process to execute simple command
void
execute_simple_command(command_t cmd, bool time_travel){
  int status;
  char **argv = cmd->u.word;

  pid_t pid = fork();
  if(pid < 0){
    //could not fork child process
    print_system_error();
  }
  else if(pid == 0)
  {
    //inside child process
    //Set IO
    IO_redirect(cmd);
    if(execvp(argv[0], argv) < 0)
    {
      //Fail to execute simple command
      print_system_error();
    }
  }
  else
  {
    //inside parent process
    //wait for child process to finish
    if( waitpid(pid, &status, 0) == -1)
    {
      print_system_error();
    }
  }

  cmd->status = WIFEXITED(status);
}

void execute_pipe_command(command_t cmd){
	//create fd for both input and output
	int fd[2];
	int read_status, write_status;
	command_t left = cmd->u.command[0];
	command_t right = cmd->u.command[1];

	//create pipe
	if (pipe(fd) < 0){
		//fail to create a pipe
		print_system_error();
	}

	pid_t write = fork();
	if (write < (pid_t) 0){
		print_system_error();
	}
	else if (write == (pid_t) 0){ //inside child process
		//copy pipe output fd to STDOUT
		dup2(fd[1], 1);
		//close output side of pipe
		close(fd[0]);
		//execute left command
		execute_command(left, false);
		//set the exit status

		exit(left->status);
	}

	pid_t read = fork();
	if (read < (pid_t) 0){
		print_system_error();
	}
	else if (read == (pid_t) 0){ //inside child process
		//copy pipe output fd to STDOUT
		dup2(fd[0], 0);
		//close output side of pipe
		close(fd[1]);
		//execute left command
		execute_command(right, false);
		//set the exit status

		exit(right->status);
	}
	close(fd[0]);
	close(fd[1]);

	waitpid(write, &write_status, 0);
	waitpid(read, &read_status, 0);

	if (!WIFEXITED(write_status) || !WIFEXITED(read_status))
		print_system_error();

	cmd->status = WEXITSTATUS(read_status);
}

void
execute_or_command(command_t cmd)
{
  command_t left = cmd->u.command[0];
  command_t right = cmd->u.command[1];

  execute_command(left,false);
  cmd->status = left->status;

  if(left->status != 0)
  {
    execute_command(right,false);
    cmd->status = right->status;
  }
}

void
execute_sequence_command(command_t cmd)
{
  command_t first = cmd->u.command[0];
  command_t second = cmd->u.command[1];

  execute_command(first, false);
  execute_command(second, false);
  cmd->status = second->status;
}

void
execute_and_command(command_t cmd){
  command_t left = cmd->u.command[0];
  command_t right = cmd->u.command[1];

  //Use false for now, as we may change it later for part c
  execute_command(left, false);
  cmd->status = left->status;

  //execute rightside command if leftside exit normally
  if(left->status == 0)
  {
    execute_command(right, false);
    cmd->status = right->status;
  }
}

void
execute_subshell_command(command_t cmd)
{
  IO_redirect(cmd);
  command_t subshell_cmd = cmd->u.subshell_command;
  execute_command(subshell_cmd, false);
  cmd->status = subshell_cmd->status;
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
  		execute_and_command(c);
  	case SEQUENCE_COMMAND: 
  		execute_sequence_command(c);
  	case OR_COMMAND:
  		execute_or_command(c);
  	case PIPE_COMMAND:
  		execute_pipe_command(c);
  	case SIMPLE_COMMAND:
  		execute_simple_command(c, false);
  	case SUBSHELL_COMMAND:
  		execute_subshell_command(c);
  	default: //Some error handling code
		error(1, 0, "Unknown error!");
  }
}


