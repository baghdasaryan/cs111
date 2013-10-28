/*
#include <stdio.h>
#include <sys/types.h>
#include <command.h>
#include <command-internal.h>

//This struct represents each node
//in the dependency graph
//it will have the following:
//-process pid executing this node
//-an array of pointers to other depended nodes
//-an array of pointers to words in this node
struct cmd_node{
	pid_t pid;
	cmd_node **dependent;
	size_t dep_size;
	command_t cmd;
};
typedef struct cmd_node *cmd_node_t;

//This struct represents each node in the dependency graph
//it simply contains the cmd node it's currently referring to and
//the next node and prev node pointer
struct dependency{
	cmd_node_t c_node;
	dependency *next;
	dependency *prev;
};
typedef struct dependency *dependency_t;
*/

