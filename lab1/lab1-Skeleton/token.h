// UCLA CS 111 Lab 1 command token interface

#include <stdbool.h>

typedef struct token *token_t;

// A likned list of tokens
struct token
{
  struct token *next;
  bool is_command;
  char* data;
};

// Add a new item to the list
void
create_token (token_t head,
              bool is_command,
              char * ch)
{
  //construct a new token 
  int ch_size = strlen(ch);
  token_t new_token = (token_t) checked_malloc(sizeof(token_t));
  new_token->is_command = is_command;
  new_token->next = NULL;
  new_token->data = (char *) checked_malloc(ch_size *sizeof(char) + 1);
  strcpy(new_token->data, ch);
  new_token->data[ch_size] = '\0';
  //insert it to the end of linked list
  if (head == NULL)
  {
    head = new_token;
  }
  else
  {
    token_t temp = head;
      while (temp->next != NULL){
        temp = temp->next;
      }
    temp->next = new_token;   
  }
}

