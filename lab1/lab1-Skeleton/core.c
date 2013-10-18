// Core functions

#include <error.h>
#include <errno.h>

// Print an error message to stderr, and exit the program
void
print_error_and_exit (size_t line_num,
                      const char *error_msg)
{
  error(1, 0, "%zd: %s", line_num, error_msg);
}

//print system call error and exit
void
print_system_error(){
	fprintf(stderr, "%s\n", stderror(errno));
	exit(1);
}
