// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "core.h"
#include "alloc.h"

//#include "parallel.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

// Get command status
int
command_status (command_t c)
{
  return !c->status;
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

// Fork a child process to execute a simple command
void
execute_simple_command (command_t cmd)
{
  int status;
  char **argv = cmd->u.word;

  pid_t pid = fork();
  if (pid < 0)  // Could not fork child process
  {
    print_system_error_and_exit();
  }
  else if (pid == 0)  // Inside the child process

  {
    // Set IO
    IO_redirect(cmd);
    if(execvp(argv[0], argv) < 0)
    {
      // Fail to execute simple command
      print_system_error_and_exit();
    }
  }
  else  // Inside the parent process
  {
    // Wait for the child to finish
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
  // Create file descriptors for both input and output
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
  else if (write == (pid_t) 0)  // Inside the child process
  {
    // Map standard output to the end of pipe
    dup2(fd[1], 1);

    close(fd[0]);
    execute_command(left);

    // Exit with left command's status
    exit(left->status);
  }

  pid_t read = fork();
  if (read < (pid_t) 0)
    print_system_error_and_exit();
  else if (read == (pid_t) 0)  // Inside the child process
  {
    dup2(fd[0], 0);

    close(fd[1]);
    execute_command(right);

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

  execute_command(left);
  cmd->status = left->status;

  // Execute rightside command if the leftside one was successful
  if (left->status == 0)
  {
    execute_command(right);
    cmd->status = right->status;
  }
}

void
execute_or_command (command_t cmd)
{
  command_t left = cmd->u.command[0];
  command_t right = cmd->u.command[1];

  execute_command(left);
  cmd->status = left->status;

  // Execute rightside command if the leftside one was not successful
  if (left->status != 0)
  {
    execute_command(right);
    cmd->status = right->status;
  }
}

void
execute_sequence_command (command_t cmd)
{
  command_t first = cmd->u.command[0];
  command_t second = cmd->u.command[1];

  execute_command(first);
  execute_command(second);
  cmd->status = second->status;
}

void
execute_subshell_command (command_t cmd)
{
  IO_redirect(cmd);
  command_t subshell_cmd = cmd->u.subshell_command;
  execute_command(subshell_cmd);
  cmd->status = subshell_cmd->status;
}

void
execute_command (command_t c)
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
      execute_simple_command(c);
      break;
    case SUBSHELL_COMMAND:
      execute_subshell_command(c);
      break;
    default:
      print_error_and_exit("Unknown command type specified.");
    }
}

//extract input/output from command_stream_t into
//an array of words
void 
get_IO (command_t cmd,
        bool only_out,
        char ***cmd_io,
        size_t *cmd_io_size)
{
  // Determine command type
  if (cmd->type == SIMPLE_COMMAND || cmd->type == SUBSHELL_COMMAND)
  {
    // Get input
    if (!only_out)
    {
      if (cmd->input != NULL)
      {
        (*cmd_io_size)++;
        *cmd_io = checked_realloc(*cmd_io, sizeof(char*) * (*cmd_io_size));

        (*cmd_io)[*cmd_io_size - 1] = cmd->input;
      }
    }
    // Get output
    if (cmd->output != NULL)
    {
      (*cmd_io_size)++;
      *cmd_io = checked_realloc(*cmd_io, sizeof(char*) * (*cmd_io_size));

      (*cmd_io)[*cmd_io_size - 1] = cmd->output;
    }
  }
  else
  {
    // Get IO for other type of commands
    get_IO(cmd->u.command[0], only_out, cmd_io, cmd_io_size);
    get_IO(cmd->u.command[1], only_out, cmd_io, cmd_io_size);
  }
}

// Check if any word from cmd1 matches any word in cmd2
bool 
depends (char **cmd1,
         char **cmd2)
{
  // Empty commands
  if (cmd1 == NULL || cmd2 == NULL)
    return false;

  int i, j;
  for (i = 0; cmd1[i] != NULL; i++)
  {
    for (j = 0; cmd2[j] != NULL; j++)
    {
      if(streq(cmd1[i],cmd2[j]))
        return true;
    }
  }

  return false;
}

int
get_num_deps (command_stream_t cmd_stream,
              int *data,
              int num_cmds)
{
  int num_deps = 0;

  int i;
  for (i = 0; i < num_cmds; i++)
  {
    if (data[i] == 0 && cmd_stream->cmd->status == -1)
      num_deps++;

    cmd_stream = cmd_stream->next;
  }

  return num_deps;
}

void
update_cmd_array(int cmd_id,
                 int num_cmds,
                 int **cmd_array,
                 int *data)
{
  int i;
  for (i = 0; i < num_cmds; i++)
  {
    if (cmd_array[i][cmd_id])
    {
      cmd_array[i][cmd_id] = 0;
      data[i]--;
    }
  }
}

void
execute_travel (command_stream_t cmd_stream,
                int num_cmds)
{
  if (num_cmds == 0)
    return;

  int **cmd_array = checked_malloc(num_cmds * sizeof(int*));

  int i;
  for(i = 0; i < num_cmds; i++)
  {
    cmd_array[i] = checked_malloc(num_cmds * sizeof(int));
    memset(cmd_array[i], 0, num_cmds * sizeof(int));
  }

  int *curr_data = checked_malloc(num_cmds *  sizeof(int));
  memset(curr_data, 0, num_cmds * sizeof(int));

  command_stream_t head, curr, cmd;
  head = curr = cmd = cmd_stream;

  int curr_id = 0, cmd_id = 0;
  while (curr != NULL)
  {
    while (cmd != curr)
    {
      char **cmd_io = checked_malloc(sizeof(char*)),
           **curr_io = checked_malloc(sizeof(char*));
      size_t cmd_io_size = 0, 
             curr_io_size = 0;

      get_IO(cmd->cmd, true, &cmd_io, &cmd_io_size);
      get_IO(curr->cmd, false, &curr_io, &curr_io_size);

      if (depends(cmd_io, curr_io))
      {
        cmd_array[curr_id][cmd_id] = 1;
        curr_data[curr_id]++;
      }

      cmd = cmd->next;
    }

    curr = curr->next;
    curr_id++;
    cmd = head;
    cmd_id = 0;
  }

  curr = head;
  int cmd_num = 0,
      pid_index = 0,
      num_deps = 0;

  while((num_deps = get_num_deps(head, curr_data, num_cmds)) != 0)
  {
    pid_index = 0;

    pid_t *pid = checked_malloc(num_deps * sizeof(pid_t));
    int *status = checked_malloc(num_deps * sizeof(int));
    int *mapping = checked_malloc(num_deps * sizeof(int));

    for (cmd_num = 0; cmd_num < num_cmds; cmd_num++)
    {
      if (curr_data[cmd_num] == 0 && curr->cmd->status == -1)
      {
        mapping[pid_index] = cmd_num;

        pid[pid_index] = fork();
        if (pid[pid_index] < 0)  // Could not fork child process
        {
          print_system_error_and_exit();
        }
        else if (pid[pid_index] == 0)  // Inside the child process
        {
          execute_command(curr->cmd);
          exit(curr->cmd->status);
        }

        pid_index++;
      }

      curr = curr->next;
    }

    for (pid_index--; pid_index > 0; pid_index--)
    {
      waitpid(pid[pid_index], &status[pid_index], 0);
    }

    curr = head;
    for (cmd_num = 0, pid_index = 0; pid_index < num_deps; pid_index++ )
    {
      while (cmd_num != mapping[pid_index])
      {
        cmd_num++;
        curr = curr->next;
      }

      curr->cmd->status = WIFEXITED(status[pid_index]);
      update_cmd_array(cmd_num, num_cmds, cmd_array, curr_data);
    }
  }
}

