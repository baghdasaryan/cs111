// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>

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

  	default: //Some error handling

  }
  error (1, 0, "command execution not yet implemented");
}
