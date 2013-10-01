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


/* =============================
   ==== Helper Functions========
   ============================= */

bool checkChar (char ch)
{
  if (isalnum(ch))
    return true;

  switch (ch)
    {
    case '.':
    case ',':
    case '-':
    case '+':
    case '%':
    case '!':
    case '/':
    case '^':
    case ':':
    case '@':
    case '_':
      return true;
      break;
    default:
      return false;
    }
}

bool is_spec_tokens(char ch){
  switch (ch){
    case ';':
    case '|':
    case '&':
    case '(':
    case ')':
    case '<':
    case '>':
      return true;
    default:
      return false;
  }
}

bool is_white_space(char ch){
  if( ch == ' ' || ch == '\n' || ch == '\t')
    return true;
  else
    return false;
}
/*===============================
  ===== Helper functions end ====
  ===============================*/

/*alloc memory for buffer */
char * buffer_mem_alloc(size_t new_size, char * buffer)
{
  //use malloc
  char * new_buffer;
  if (buffer == NULL)
  {
    new_buffer = (char *) malloc (new_size);
  }
  else
  {
    new_buffer = (char *) realloc (buffer, new_size);
  }
  return new_buffer;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

  // Start to read file stream
  command_stream_t first_stream;
  char cur_char;
  size_t buffer_size = 0;
  do {
    char *buffer;
    cur_char = (*get_next_byte)(get_next_byte_argument);
    //A Word
    if (checkChar(cur_char)){
      buffer_size++;
      buffer = buffer_mem_alloc(buffer_size,buffer);
      buffer[buffer_size-1] = cur_char;
    }
    else if(is_spec_tokens(cur_char)){
      /*do something with tokens*/
    }
    else if(is_white_space(cur_char)){
      /*do something with white space, special case for newline */
    }
    else if(cur_char == '#'){
      /*do something to remove comments */
    }
    else{
      /* handle errpr */
    }

  } while (cur_byte != EOF);
  /*error (1, 0, "command reading not yet implemented");
  return 0;*/
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
