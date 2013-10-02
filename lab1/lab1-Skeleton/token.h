// Command token interface

#include "alloc.h"

#include <stdbool.h>
#include <string.h>

typedef struct token *token_t;

// A data structure to represent a likned list of tokens
struct token
{
  struct token *next;
  bool is_command;
  char* data;
};

// Add a new item to the list
void
create_token (token_t head,
              token_t current,
              bool is_command,
              char *token)
{
  // Allocate a new node
  token_t temp = (token_t) checked_malloc(sizeof(token_t));

  // Add data to the node
  temp->next = NULL;
  temp->is_command = is_command;
  temp->data = (char*) checked_malloc(strlen(token) * sizeof(char));
  strcpy(temp->data, token);

  // Insert the new node at the end of the linked list
  if (head == NULL)
  {
    head = temp;
  }
  else
  {
    current->next = temp;
  }
  current = temp;
}

