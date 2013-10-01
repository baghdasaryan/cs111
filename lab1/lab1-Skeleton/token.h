

typedef struct token *token_t;

struct token
{
  char* data;
  struct token *next;
  bool is_command;
};



