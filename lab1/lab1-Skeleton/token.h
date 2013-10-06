// Command token interface

#include "alloc.h"

#include <stdbool.h>
#include <string.h>

typedef struct token *token_t;

// A data structure to represent a likned list of tokens
struct token
{
  struct token *next;
  bool is_special_command;
  char* data;
};

// Add a new item to the list
void
create_token (token_t *head_token,
              token_t *current_token,
              bool is_special_command,
              const char *token)
{
  // Allocate a new node
  token_t temp = (token_t) checked_malloc(sizeof(struct token));

  // Add data to the node
  temp->next = NULL;
  temp->is_special_command = is_special_command;
  temp->data = (char*) checked_malloc(strlen(token) * sizeof(char));
  strcpy(temp->data, token);

  // Insert the new node at the end of the linked list
  if (*head_token == NULL)
  {
    *head_token = temp;
  }
  else
  {
    (*current_token)->next = temp;
  }
  *current_token = temp;
}

