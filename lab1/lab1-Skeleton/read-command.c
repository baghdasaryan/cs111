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

// Returns the next character
void
get_next_char (int (*get_next_byte) (void *), 
               void *get_next_byte_argument,
               char *ch)
{
  *ch = (char) get_next_byte(get_next_byte_argument);
}

// Returns the next character that is neither white-space nor TAB
void 
get_next_non_empty_char (int (*get_next_byte) (void *), 
                         void *get_next_byte_argument,
                         char *ch)
{
  do {
    get_next_char(get_next_byte, get_next_byte_argument, ch);
  } while(*ch == ' ' || *ch == '\t');
}

// Checks if the next argument is next_char
bool
check_next_char (int (*get_next_byte) (void *), 
                 void *get_next_byte_argument,
                 char next_char)
{
  return ((char) get_next_byte(get_next_byte_argument) == next_char);
}

// Checks if the given character belongs to a word rather than to a command
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

// Reads a complete word
void
get_word (int (*get_next_byte) (void *),
          void *get_next_byte_argument,
          char *ch,
          char **command)
{
  int num_chars = 256;
  int num_used_chars = 0;
  char *buffer = (char*) checked_malloc(num_chars * sizeof(char));

  while (is_word(*ch))
  {
    // Dynamically reallocate array when necessary
    if (num_used_chars >= num_chars - 1)
    {
      num_chars *= 2;
      buffer = (char*) checked_realloc(buffer, num_chars * sizeof(char));
      if (buffer == NULL)
      {
        error (1, 0, "Error reallocating memory");
      }
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

// TODO: Finish writing this function
command_t
gen_command_tree ()
{

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

  // Build a linked list of tokens

  char ch;
  get_next_non_empty_char(get_next_byte, get_next_byte_argument, &ch);

  int line_number;
  for (line_number = 1; ch != EOF; line_number++)
  {
    if (is_word(ch)) // process words
    {
      char *tmp = NULL;
      get_word(get_next_byte, get_next_byte_argument, &ch, &tmp);
      create_token(tokens_head, current_token, false, tmp);
      free(tmp);
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
            error(1, 0, "%d: ERR", line_number);
          }      
          break;
        case '\n': // End of line
          create_token(tokens_head, current_token, true, "\n\0");
          break;
        default:
          error(1, 0, "%d: ERR", line_number);
        }
    }

    get_next_non_empty_char(get_next_byte, get_next_byte_argument, &ch);
  }

  // Construct trees for command streams by iterating through the linked list
  // with tokens
 
  command_t command_stream_head = NULL,
            current_command_stream = NULL;
  while (current_token != NULL)
  {
    // Create a tree for a set of commands
    command_t command_tree = gen_command_tree(current_token); // TODO: Update this function

    // Allocate a new node
    command_stream_t temp = checked_malloc(sizeof(command_t ));

    // Add data to the node
    temp->next = NULL;
    temp->cmd = NULL; // TODO: Get the command tree here instead of NULL

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

