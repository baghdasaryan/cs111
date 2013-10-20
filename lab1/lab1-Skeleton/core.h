// Core functions

#ifndef CORE_H
#define CORE_H

#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void print_error_and_exit (const char *error_msg);
void print_parsing_error_and_exit (size_t line_num, const char *error_msg);
void print_system_error_and_exit (void);
bool streq (const char *str1, const char *str2);

#endif  // CORE_H

