#include "core.h"

// Print error message to stderr and exit
void
print_error_and_exit (const char *error_msg)
{
  error(1, 0, "%s", error_msg);
}

// Print line number and error message to stderr and exit
void
print_parsing_error_and_exit (size_t line_num,
                      const char *error_msg)
{
  error(1, 0, "%zd: %s", line_num, error_msg);
}

// Print system call error and exit
void
print_system_error_and_exit (void)
{
  fprintf(stderr, "%s\n", strerror(errno));
  exit(1);
}

// Check if two character strings are equal
bool
streq (const char *str1,
       const char *str2)
{
  return (strcmp(str1, str2) == 0);
}

