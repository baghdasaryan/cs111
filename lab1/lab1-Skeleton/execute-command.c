// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "core.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// Get command status
int
command_status (command_t c)
{
  return c->status;
}

void
IO_redirect (command_t c)
{
  if (c->input != NULL)
  {
    int fd = open(c->input, O_RDONLY);
    if (fd < 0)
      print_system_error_and_exit();
    if (dup2(fd, STDIN_FILENO) < 0)
      print_system_error_and_exit();
    if (close(fd) < 0)
      print_system_error_and_exit();
  }
  else if (c->output != NULL)
  {
    int fd = open(c->output, O_CREAT | O_WRONLY);
    if (fd < 0)
      print_system_error_and_exit();
    if (dup2(fd, STDOUT_FILENO) <0)
      print_system_error_and_exit();
    if (close(fd) < 0)
      print_system_error_and_exit();
  }
}

//fork a child process to execute simple command
void
execute_simple_command (command_t cmd,
                        bool time_travel)
{
  int status;
  char **argv = cmd->u.word;

  pid_t pid = fork();
  if (pid < 0)
  {
    //could not fork child process
    print_system_error_and_exit();
  }
  else if (pid == 0)
  {
    //inside child process
    //Set IO
    IO_redirect(cmd);
    if(execvp(argv[0], argv) < 0)
    {
      //Fail to execute simple command
      print_system_error_and_exit();
    }
  }
  else
  {
    //inside parent process
    //wait for child process to finish
    if (waitpid(pid, &status, 0) == -1)
    {
      print_system_error_and_exit();
    }
  }

  cmd->status = WIFEXITED(status);
}

void
execute_pipe_command (command_t cmd)
{
  //create fd for both input and output
  int fd[2];
  int read_status, write_status;
  command_t left = cmd->u.command[0];
  command_t right = cmd->u.command[1];

  // Create pipe
  if (pipe(fd) < 0)
    print_system_error_and_exit();

  pid_t write = fork();
  if (write < (pid_t) 0)
    print_system_error_and_exit();
  else if (write == (pid_t) 0)  // child
  {
    // Map standard output to the end of pipe
    dup2(fd[1], 1);

    close(fd[0]);
    execute_command(left, false);

    // Exit with left command's status
    exit(left->status);
  }

  pid_t read = fork();
  if (read < (pid_t) 0)
    print_system_error_and_exit();
  else if (read == (pid_t) 0)  // child
  {
    dup2(fd[0], 0);

    close(fd[1]);
    execute_command(right, false);

    // Exit with right command's status
    exit(right->status);
  }

  // Close descriptors
  close(fd[0]);
  close(fd[1]);

  // Wait for children
  waitpid(write, &write_status, 0);
  waitpid(read, &read_status, 0);

  if (!WIFEXITED(write_status) || !WIFEXITED(read_status))
    print_system_error_and_exit();

  cmd->status = WEXITSTATUS(read_status);
}

void
execute_and_command (command_t cmd)
{
  command_t left = cmd->u.command[0];
  command_t right = cmd->u.command[1];

  execute_command(left, false);
  cmd->status = left->status;

  // Execute rightside command if the leftside one was successful
  if (left->status == 0)
  {
    execute_command(right, false);
    cmd->status = right->status;
  }
}

void
execute_or_command (command_t cmd)
{
  command_t left = cmd->u.command[0];
  command_t right = cmd->u.command[1];

  execute_command(left,false);
  cmd->status = left->status;

  // Execute rightside command if the leftside one was not successful
  if (left->status != 0)
  {
    execute_command(right,false);
    cmd->status = right->status;
  }
}

void
execute_sequence_command (command_t cmd)
{
  command_t first = cmd->u.command[0];
  command_t second = cmd->u.command[1];

  execute_command(first, false);
  execute_command(second, false);
  cmd->status = second->status;
}

void
execute_subshell_command (command_t cmd)
{
  IO_redirect(cmd);
  command_t subshell_cmd = cmd->u.subshell_command;
  execute_command(subshell_cmd, false);
  cmd->status = subshell_cmd->status;
}

void
execute_command (command_t c,
                 bool time_travel)
{
  switch (c->type)
    {
    case AND_COMMAND: 
      execute_and_command(c);
      break;
    case SEQUENCE_COMMAND: 
      execute_sequence_command(c);
      break;
    case OR_COMMAND:
      execute_or_command(c);
      break;
    case PIPE_COMMAND:
      execute_pipe_command(c);
      break;
    case SIMPLE_COMMAND:
      execute_simple_command(c, false);
      break;
    case SUBSHELL_COMMAND:
      execute_subshell_command(c);
      break;
    default:
      print_error_and_exit("Unknown command type specified.");
    }
}

