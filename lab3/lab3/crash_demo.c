#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <inttypes.h>

#include "ospfs.h"


// Function declarations
void print(char *data);
void print_and_sleep(char *data, float time);

// Global variable
static int silent = 0;


// Main function
int main(int argc, char *argv[])
{
	int ret = 0;

	// Test input
	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Usage: crash_demo NUMBER_OF_WRITES [SILENT]\n");
		ret = 1;
	} else if (atoi(argv[1]) < -1) {
		fprintf(stderr, "NUMBER_OF_WRITES has to be >= -1\n");
		ret = 1;
	} else if (argc == 3 && (strcmp(argv[2], "silent") != 0 && strcmp(argv[2], "SILENT") != 0)) {
		fprintf(stderr, "Second argument, if any, has to be SILENT\n");
		ret = 1;
	} else {
		if (argc == 3)
			silent = 1;
		ret = crash_test(atoi(argv[1]));
	}

	exit(ret);
}



/* ***************************************** */
/*         CRASH TEST DEMO FUNCTIONS         */
/* ***************************************** */

int crash_test(int num_writes)
{
	int status = 0,
	    file_descriptor = 0;
	print_and_sleep("This program tests file system robustness by crushing the latter one.\n", 1);

	if (!silent) {
		fprintf(stdout, "Setting up the program with nwrites_to_crash=%d\n", num_writes);
		sleep(1);
	}

	file_descriptor = open("./test/hello.txt", O_RDONLY);
	status = ioctl(file_descriptor, CRASHER, num_writes);

	print_and_sleep("Setup completed.\n", 1);

	print("Cleaning up.\n");
	close(file_descriptor);

	return status;
}

/* ***************************************** */



/* ***************************************** */
/*           PRINT FUNCTIONS                 */
/* ***************************************** */

void print(char *data)
{
	if (!silent)
		fprintf(stdout, data);
}

void print_and_sleep(char *data, float time)
{
	if (!silent) {
		fprintf(stdout, data);
		sleep(time);
	}
}

/* ***************************************** */

