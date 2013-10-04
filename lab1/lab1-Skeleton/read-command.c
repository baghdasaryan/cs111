// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include "token.h"

#include <ctype.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct command_stream
{
  command_stream_t next;
  command_stream_t prev;
  command_t cmd;
};


// ********************************** //
// ***                            *** //
// ***   Helper Functions Begin   *** //
// ***                            *** //
// ********************************** //

// Return the next character
void
get_next_char (int (*get_next_byte) (void *), 
               void *get_next_byte_argument,
               char *ch)
{
  *ch = (char) get_next_byte(get_next_byte_argument);
}

// Return the next character that is neither white-space nor TAB
void 
get_next_non_empty_char (int (*get_next_byte) (void *), 
                         void *get_next_byte_argument,
                         char *ch)
{
  do {
    get_next_char(get_next_byte, get_next_byte_argument, ch);
  } while (*ch == ' ' || *ch == '\t');
}

// Check if the next argument is next_char
bool
check_next_char (int (*get_next_byte) (void *), 
                 void *get_next_byte_argument,
                 char next_char)
{
  return ((char) get_next_byte(get_next_byte_argument) == next_char);
}

// Check if the given character belongs to a word rather than to a command
bool
is_word (char ch)
{
  if (isalnum(ch))
    return true;

  switch (ch)
    {
    case '!':
    case '%':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case '@':
    case '^':
    case '_':
      return true;
      break;
    default:
      return false;
    }
}

// Read a complete word
void
get_word (int (*get_next_byte) (void *),
          void *get_next_byte_argument,
          char *ch,
          char **command)
{
  size_t num_chars = 256;
  size_t num_used_chars = 0;
  char *buffer = (char*) checked_malloc(num_chars * sizeof(char));

  while (is_word(*ch))
  {
    // Dynamically reallocate the array (when necessary)
    if (num_used_chars >= num_chars - 1)
    {
      num_chars *= 2;
      buffer = (char*) checked_realloc(buffer, num_chars * sizeof(char));
    }

    buffer[num_used_chars] = *ch;
    get_next_char(get_next_byte, get_next_byte_argument, ch);
    num_used_chars++;
  }
  buffer[num_used_chars] = '\0';
  num_used_chars++;

  *command = (char*) checked_malloc(num_used_chars * sizeof(char));
  strcpy(*command, buffer);
  free(buffer);
}



void
print_error_and_exit(size_t line_num,
                     const char *error)
{
  error(1, 0, "%d: %s", line_number, error);
}

bool
streq (const char *str1,
       const char *str2)
{
  return (strcmp(str1, str2) == 0);
}

// TODO: Finish writing this function
// Generate a tree of commands to be saved in command stream (for further
// execution)
command_t
gen_command_tree (token_t token, size_t *line_num)
{
  command_t cmd = NULL;

  // Ignore empty lines
  while (token != NULL && streq(token->data, "\n"))
  {
    token = token->next;
    (*line_num)++;
  }

  if (token == NULL)
    return NULL;

  // Process token
  if (!token->is_special_command)
  {
    cmd = create_simple_command(token);
  }
  else if (streq(token->data, "("))
  {
    // Process a subshell command
    token = token->next;
    command_t subshell_cmds = gen_command_tree(token, line_num);

    if (subshell_cmds == NULL)
      print_error_and_exit(*line_num, "GIVE ME A NAME :P .");
    else if (streq(token->data, ")"))
      print_error_and_exit(*line_num, "GIVE ME A NAME :P .");

    cmd = create_subshell_command(subshell_cmds);
    token = token->next;
  }
  else
  {
    print_error_and_exit(*line_num, "GIVE ME A NAME :P .");
  }

  if (token != NULL && streq(token->data, "<"))
  {
    if (!token->is_special_command)
    {
      cmd->input = token->data;
      token = token->next;
    }
    else
      print_error_and_exit(*line_num, "GIVE ME A NAME :P .");
  }

  if (token != NULL && streq(token->data, ">"))
  {
    if (!token->is_special_command)
    {
      cmd->output = token->data;
      token = token->next;
    }
    else
      print_error_and_exit(*line_num, "GIVE ME A NAME :P .");
  }

  if (token != NULL || streq(token->data, ")"))
    return cmd;
  else if (streq(token->data, "\n"))
  {
    token = token->next;
    (*line_num)++;
    return cmd;
  }

  command_t cmd2 = checked_malloc(sizeof(struct command));
  enum command_type cmd2_type;

  if (streq(token->data, "&&"))
    cmd2_type = AND_COMMAND;
  else if (streq(token->data, ";"))
    cmd2_type = SEQUENCE_COMMAND;
  else if (streq(token->data, "||"))
    cmd2_type = OR_COMMAND;
  else if (streq(token->data, "|"))
    cmd2_type = PIPE_COMMAND;
  else
    print_error_and_exit(*line_num, "GIVE ME A NAME :P .");

  token = token->next;
  cmd2 = gen_command_tree(token, line_num);

  if (cmd2 == NULL)
    print_error_and_exit(*line_num, "GIVE ME A NAME :P .");
  

  return arrange_tree_nodes(cmd, cmd2, type);
}

command_t
arrange_tree_nodes (command_t cmd1,
                    command_t cmd2,
                    enum command_type type)
{
  command_t tmp = checked_malloc(sizeof(struct command));

  tmp->type = type;
  tmp->status = -1;

  if (type == SUBSHELL_COMMAND)
  {
    tmp->u.subshell_command = cmd1;
  }
  else
  {
    tmp->u.command[0] = cmd1;
    tmp->u.command[1] = cmd2;
  }

  return tmp;
}

command_t
arrange_tree_nodes (command_t cmd1,
                    command_t cmd2,
                    enum command_type type)
{
  if (cmd_type == SEQUENCE_COMMAND || cmd_type == PIPE_COMMAND ||
      cmd2 == NULL || cmd2->type == SUBSHELL_COMMAND ||
      cmd2->type == SIMPLE_COMMAND)
  {

  }
  else
  {
    arrange_tree_nodes((cmd1, cmd2->u.command[0]), cmd2->u.command[1], cmd2->type);
  }
}

// FIXME: DONE!!!
// Create a simple command
command_t
create_simple_command (token_t token)
{
  // Allocate memory for the command
  command_t simple_command = (command_t) checked_malloc(sizeof(struct command));

  // Add data to the command
  simple_command->type = SIMPLE_COMMAND;
  simple_command->status = -1;
  simple_command->input = NULL;
  simple_command->output = NULL;

  // Add command tokens
  size_t num_words = 0;
  char **words = NULL;
  while (token != NULL && !token->is_special_command)
  {
    // Reallocate memory to add a new command token
    words = checked_realloc(words, (num_words + 1) * sizeof(char *)); 

    words[num_words++] = token->data;
    token = token->next;
  }

  words[num_words] = NULL;
  command->u.word = words;

  return simple_command;
}

// FIXME: DONE!!!
// Create a subshell command
command_t
create_subshell_command (command_t commands){
  // Allocate memory for the command
  command_t subshell_command = checked_malloc(sizeof(struct command));

  // Add data to the command
  subshell_command->type = SUBSHELL_COMMAND;
  subshell_command->status = -1;
  subshell_command->input = NULL;
  subshell_command->output = NULL;
  subshell_command->u.subshell_command = commands;

  return subshell_command;
}

//Generate rest of the commands
command_t
gen_complete_command (enum command_type cmd_type, command_t cmd_a, command_t cmd_b){
  command_t complete_command = NULL;
  complete_command = checked_malloc(sizeof(command_t));
  complete_command-> type = cmd_type;
  complete_command-> status = -1;
  complete_command-> input = NULL;
  complete_command-> output = NULL; 
  complete_command-> command[0] = cmd_a;
  complete_command-> command[1] = cmd_b;
  return complete_command;
}

// ******************************** //
// ***                          *** //
// ***   Helper Functions End   *** //
// ***                          *** //
// ******************************** //


// TODO: Performance could be potentially improved if we could start
//         constructing the command stream tree right away (without first
//         saving tokens)
command_stream_t 
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  token_t tokens_head = NULL,
          current_token = NULL;

  // ***************************** //
  // Build a linked list of tokens //
  // ***************************** //
  char ch;
  get_next_non_empty_char(get_next_byte, get_next_byte_argument, &ch);

  size_t line_number;
  for (line_number = 1; ch != EOF; line_number++)
  {
    if (is_word(ch)) // process words
    {
      char *word = NULL;
      get_word(get_next_byte, get_next_byte_argument, &ch, &word);
      create_token(tokens_head, current_token, false, word);
      free(word);
      if (ch != ' ' && ch != '\t')
      {
        continue;
      }
    }
    else // process commands
    {
      switch (ch)
        {
        case '#':  // Comment
          // Ignore line
          while (ch != '\n' && ch != EOF)
            get_next_char(get_next_byte, get_next_byte_argument, &ch);
          break;
        case '(':  // Start subshell
          create_token(tokens_head, current_token, true, "(\0");
          break;
        case ')':  // End subshell
          create_token(tokens_head, current_token, true, "(\0");
          break;
        case '<':  // Redirect output
          create_token(tokens_head, current_token, true, "<\0");
          break;
        case '>':  // Read from
          create_token(tokens_head, current_token, true, ">\0");
          break;
        case ';':  // End command sequence
          create_token(tokens_head, current_token, true, ";\0");
          break;
        case '|':  // OR command
          if (check_next_char(get_next_byte, get_next_byte_argument, '|'))
          {
            create_token(tokens_head, current_token, true, "||\0");
          }
          else
          {
            create_token(tokens_head, current_token, true, "|\0");
            if (ch != ' ' && ch != '\t')
            {
              continue;
            }
          }      
          break;
        case '&':  // AND command
          if (check_next_char(get_next_byte, get_next_byte_argument, '&'))
          {
            create_token(tokens_head, current_token, true, "&&\0");
          }
          else
          {
            print_error_and_exit(line_number, "GIVE ME A NAME :P .");
          }      
          break;
        case '\n': // End of line
          create_token(tokens_head, current_token, true, "\n\0");
          break;
        default:
          print_error_and_exit(line_number, "GIVE ME A NAME :P .");
        }
    }

    get_next_non_empty_char(get_next_byte, get_next_byte_argument, &ch);
  }

  // ******************************************************************* //
  // Construct trees for command streams by iterating through the linked //
  // list of tokens                                                      //
  // ******************************************************************* //
  command_t command_stream_head = NULL,
            current_command_stream = NULL;

  current_token = tokens_head;
  line_number = 1;
  while (current_token != NULL)
  {
    // Create a tree for of commands
    command_t command_tree = NULL;
    if ((command_tree = gen_command_tree(current_token, &line_number)) == NULL);
      break;

    // Allocate a new node
    command_stream_t temp = checked_malloc(sizeof(struct command_stream));

    // Add data to the node
    temp->next = NULL;
    temp->cmd = command_tree;

    // Add node to the list
    if (command_stream_head == NULL)
    {
      command_stream_head = temp;
      temp->prev = NULL;
    }
    else
    {
      current_command_stream->next = temp;
      temp->prev = current_command_stream;
    }
    current_command_stream = temp;
  }

  return command_stream_head;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error(1, 0, "command reading not yet implemented");
  return 0;
}

