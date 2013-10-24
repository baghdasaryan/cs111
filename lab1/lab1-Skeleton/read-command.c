// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include "token.h"
#include "core.h"

#include <ctype.h>

struct command_stream
{
  command_stream_t next;
  command_stream_t prev;
  command_t cmd;
};

typedef struct token *token_t;


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
                 char *ch,
                 char next_char)
{
  get_next_char(get_next_byte, get_next_byte_argument, ch);
  return (*ch == next_char);
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

    buffer[num_used_chars++] = *ch;
    get_next_char(get_next_byte, get_next_byte_argument, ch);
  }
  buffer[num_used_chars++] = '\0';

  *command = (char*) checked_malloc(num_used_chars * sizeof(char));
  strcpy(*command, buffer);
  free(buffer);
}

// Generate a command for SEQUENCE_COMMAND, SIMPLE_COMMAND, PIPE_COMMAND, and
// SUBSHELL_COMMAND types
command_t
create_command (command_t cmd1,
                command_t cmd2,
                enum command_type cmd_type)
{
  // Allocate memory for the command
  command_t new_cmd = checked_malloc(sizeof(struct command));

  // Add data to the command
  new_cmd->type = cmd_type;
  new_cmd->status = -1;
  new_cmd->input = NULL;
  new_cmd->output = NULL;

  // Assign nodes based on the command type
  if (cmd_type == SUBSHELL_COMMAND)
  {
    new_cmd->u.subshell_command = cmd1;
  }
  else
  {
    new_cmd->u.command[0] = cmd1;
    new_cmd->u.command[1] = cmd2;
  }

  return new_cmd;
}

// "Sort" commands based on precedence
command_t
set_precedence (command_t cmd1,
                command_t cmd2,
                enum command_type cmd_type)
{
  // Create commands based on precedence
  if (cmd_type == SEQUENCE_COMMAND || cmd2 == NULL || 
      cmd2->type == SUBSHELL_COMMAND || cmd2->type == SIMPLE_COMMAND)
  {
    return (create_command(cmd1, cmd2, cmd_type));
  }
  else if (cmd_type == AND_COMMAND || cmd_type == OR_COMMAND)
  {
    if (cmd2->type == PIPE_COMMAND)
      return (create_command(cmd1, cmd2, cmd_type));
    else
      return (create_command(set_precedence(cmd1, cmd2->u.command[0], cmd_type),
                             cmd2->u.command[1],
                             cmd2->type));
  }
  else
  {
    return (create_command(set_precedence(cmd1, cmd2->u.command[0], cmd_type),
                           cmd2->u.command[1],
                           cmd2->type));
  }
}

// Create a simple command
command_t
create_simple_command (token_t *token)
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
  while (*token != NULL && !(*token)->is_special_command)
  {
    // Reallocate memory to add a new command token
    words = checked_realloc(words, (num_words + 1) * sizeof(char *)); 

    words[num_words++] = (*token)->data;
    *token = (*token)->next;
  }
  words[num_words] = NULL;

  simple_command->u.word = words;

  return simple_command;
}

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

// Return command type of the token, or exit with an error if incorrect token
enum command_type
get_command_type(token_t token,
                 size_t line_num)
{
  enum command_type cmd_type;

  if (streq(token->data, "&&"))
    cmd_type = AND_COMMAND;
  else if (streq(token->data, ";"))
    cmd_type = SEQUENCE_COMMAND;
  else if (streq(token->data, "||"))
    cmd_type = OR_COMMAND;
  else if (streq(token->data, "|"))
    cmd_type = PIPE_COMMAND;
  else
    print_parsing_error_and_exit(line_num, "Unknown command type.");

  return cmd_type;
}

// Set command input
void
set_command_input (command_t cmd,
                   token_t *token,
                   size_t line_num)
{
  if (*token!= NULL && streq((*token)->data, "<"))
  {
    *token = (*token)->next;
    if (*token!= NULL && !(*token)->is_special_command)
    {
      cmd->input = (*token)->data;
      *token = (*token)->next;
    }
    else
      print_parsing_error_and_exit(line_num, "Cannot read input from a command.");
  }
}

// Set command output
void
set_command_output (command_t cmd,
                    token_t *token,
                    size_t line_num)
{
  if (*token != NULL && streq((*token)->data, ">"))
  {
    *token = (*token)->next;
    if (*token!= NULL && !(*token)->is_special_command)
    {
      cmd->output = (*token)->data;
      *token = (*token)->next;
    }
    else
      print_parsing_error_and_exit(line_num, "Cannot write output to a command.");
  }
}

// Generate a tree of commands to be saved in command stream (for further
// execution)
command_t
gen_command_tree (token_t *token, size_t *line_num, size_t * num_subshell)
{
  command_t cmd = NULL;

  // Ignore empty lines
  while (*token != NULL && streq((*token)->data, "\n"))
  {
    *token = (*token)->next;
    (*line_num)++;
  }

  if (*token == NULL)
    return NULL;

  // Process token
  if (!(*token)->is_special_command)
  {
    cmd = create_simple_command(token);
  }
  else if (streq((*token)->data, "("))
  {
    (*num_subshell)++;
    // Process a subshell command
    *token = (*token)->next;
    command_t subshell_cmds = gen_command_tree(token, line_num, num_subshell);

    if (subshell_cmds == NULL)
      print_parsing_error_and_exit(*line_num, "Failed to generate a command tree for subshell command(s).");
    else if (!streq((*token)->data, ")"))
      print_parsing_error_and_exit(*line_num, "Please make sure that subshell command(s) is/are correct.");

    cmd = create_subshell_command(subshell_cmds);
    *token = (*token)->next;
  }
  else
  {
    print_parsing_error_and_exit(*line_num, "Unrecognized command.");
  }

  set_command_input(cmd, token, *line_num);
  set_command_output(cmd, token, *line_num);

  if (*token == NULL || streq((*token)->data, ")"))
  {
    (*num_subshell)--;
    if (*num_subshell != 0)
    {
      print_parsing_error_and_exit(*line_num, "Parantheses mismatch.");
    }
    return cmd;
  }
  else if (streq((*token)->data, "\n"))
  {
    *token = (*token)->next;
    (*line_num)++;
    if (*num_subshell != 0)
    {
      print_parsing_error_and_exit(*line_num, "Parantheses mismatch.");
    }
    return cmd;
  }

  enum command_type cmd2_type = get_command_type(*token, *line_num);

  command_t cmd2 = checked_malloc(sizeof(struct command));

  *token = (*token)->next;
  cmd2 = gen_command_tree(token, line_num, num_subshell);

  if (cmd2 == NULL)
    print_parsing_error_and_exit(*line_num, "Failed to generate a command tree.");

  return set_precedence(cmd, cmd2, cmd2_type);
}

// *********************************** //
// ***                             *** //
// ***   End of Helper Functions   *** //
// ***                             *** //
// *********************************** //


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
  for (line_number = 1; ch != EOF;)
  {
    if (is_word(ch)) // process words
    {
      char *word = NULL;
      get_word(get_next_byte, get_next_byte_argument, &ch, &word);
      create_token(&tokens_head, &current_token, false, word);
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
          create_token(&tokens_head, &current_token, true, "(\0");
          break;
        case ')':  // End subshell
          create_token(&tokens_head, &current_token, true, ")\0");
          break;
        case '<':  // Redirect output
          create_token(&tokens_head, &current_token, true, "<\0");
          break;
        case '>':  // Read from
          create_token(&tokens_head, &current_token, true, ">\0");
          break;
        case ';':  // End command sequence
          create_token(&tokens_head, &current_token, true, ";\0");
          break;
        case '|':  // OR command
          if (check_next_char(get_next_byte, get_next_byte_argument, &ch, '|'))
          {
            create_token(&tokens_head, &current_token, true, "||\0");
          }
          else
          {
            create_token(&tokens_head, &current_token, true, "|\0");
            if (ch != ' ' && ch != '\t')
            {
              continue;
            }
          }      
          break;
        case '&':  // AND command
          if (check_next_char(get_next_byte, get_next_byte_argument, &ch, '&'))
          {
            create_token(&tokens_head, &current_token, true, "&&\0");
          }
          else
          {
            print_parsing_error_and_exit(line_number, "Unknown token, should be &&.");
          }
          break;
        case '\n': // End of line
          line_number++;
          create_token(&tokens_head, &current_token, true, "\n\0");
          break;
        default:
          print_parsing_error_and_exit(line_number, "Unknown token.");
        }
    }

    get_next_non_empty_char(get_next_byte, get_next_byte_argument, &ch);
  }


  // ******************************************************************* //
  // Construct trees for command streams by iterating through the linked //
  // list of tokens                                                      //
  // ******************************************************************* //
  command_stream_t command_stream_head = NULL,
                   current_command_stream = NULL;

  current_token = tokens_head;
  line_number = 1;
  while (current_token != NULL)
  {
    size_t * num_subshell = checked_malloc(sizeof(size_t));
    *num_subshell = 0;
    // Create a tree for of commands
    command_t command_tree = NULL;
    if ((command_tree = gen_command_tree(&current_token, &line_number, num_subshell)) == NULL)
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
read_command_stream (command_stream_t *s)
{
  //return each command from command_tree
  if (*s != NULL){
    // Free previous command stream node
    if ((*s)->prev != NULL){
      free((*s)->prev->cmd);
      free((*s)->prev);
    }
    command_t tmp = (*s)->cmd;

    // Advance command_stream pointer
    *s = (*s)->next;

    return tmp;
  }

  return NULL;
}

