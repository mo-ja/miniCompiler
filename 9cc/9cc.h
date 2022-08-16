#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum{
	     TK_RESERVED,
	     TK_NUM,
	     TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

Token *token;
char *user_input;

typedef enum{
        ND_ADD,
        ND_SUB,
        ND_MUL,
        ND_DIV,
        ND_NUM,
        ND_EQ,
        ND_NE,
        ND_LT,
        ND_LE,
} NodeKind;


struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

void error(char *fmt);
void error_at(char *loc, char *fmt, ...);
bool consume(char* op);
void expect(char* op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

typedef struct Node Node;

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void gen(Node *node);

Node *expr();
Node *primary();
Node *unary();
Node *equality();
Node *relational();
Node *add();
Node *mul();


