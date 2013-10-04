// Core functions

#include <error.h>

// Print an error message to stderr, and exit the program
void
print_error_and_exit (size_t line_num,
                      const char *error_msg)
{
  error(1, 0, "%zd: %s", line_num, error_msg);
}
