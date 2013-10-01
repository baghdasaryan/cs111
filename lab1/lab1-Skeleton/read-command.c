// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <ctype.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
struct command_stream
{
  struct command_stream* next;
  struct command_stream* prev;
  struct command* curr;
};

/* ========================
   ==  Helper Functions  ==
   ======================== */

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
get_next_non-empty_char (int (*get_next_byte) (void *), 
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

// Checks if the given character can belong to a command
bool
is_command (char ch)
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

void
get_command (int (*get_next_byte) (void *),
             void *get_next_byte_argument,
             char *ch,
             char **command)
{
  char buffer[256];
  int num_used_bytes = 0;

  while (is_command(*ch))
  {
    buffer[num_used_bytes] = *ch;
    get_next_char(get_next_byte, get_next_byte_argument, ch);
    num_used_bytes++;
  }
  buffer[num_used_bytes] = '\0';

  *command = checked_malloc(sizeof(char) * num_used_bytes + 1);
  strcpy(*temp, buffer);
}

/*===============================
  ===== Helper functions end ====
  ===============================*/

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  char ch;
  get_next_non-empty_char(get_next_byte, get_next_byte_argument, &ch);

  for (int line_number = 1; ch != EOF; line_number++)
  {
    if (is_command(ch))
    {
      char *tmp = NULL;
      get_command(get_next_byte, get_next_byte_argument, &ch, &tmp);

      free(tmp);
    }
    else
    {
      switch(ch)
        {
        case '#':  // Comment
          // Ignore line
          while (ch != '\n' && ch != EOF)
            get_next_char(get_next_byte, get_next_byte_argument, &ch);
          break;
        case '(':  // Start subshell
      
          break;
        case ')':  // End subshell
      
          break;
        case '<':  // Redirect output
      
          break;
        case '>':  // Read from
      
          break;
        case ';':  // End command sequence
      
          break;
        case '|':  // OR command
          if(check_next_char(get_next_byte, get_next_byte_argument, '|'))
          {
              
          }
          else
          {
              
          }      
          break;
        case '&':  // AND command
          if(check_next_char(get_next_byte, get_next_byte_argument, '&'))
          {
              
          }
          else
          {
            error (1, 0, "%d: ERR", line_number);
          }      
          break;
        case '\n': // End of line
      
          break;
      
        default:
          error (1, 0, "%d: ERR", line_number);
        }
    }

    get_next_non-empty_char(get_next_byte, get_next_byte_argument, &ch);

  }


  // Construct a command tree

  error (1, 0, "command reading not yet implemented");
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
