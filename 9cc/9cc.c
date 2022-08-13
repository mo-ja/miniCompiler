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
};

Token *token;
char *user_input;

void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...){
  va_list ap;
  int pos = loc - user_input;

  va_start(ap, fmt);
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;

  token = token->next;
  return true;
}

void expect(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "%c is expected but not detected.", op);
  token = token->next;
  return;
}

int expect_number(){
  if (token->kind != TK_NUM)
    error_at(token->str, "%c is not a number.", token->str[0]);
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){
    if (isspace(*p)){
      p++;
      continue;
    }
    if(*p == '+' || *p == '-' ){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error_at(p, "cannot be tokenized.");
  } 
  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char** argv)
{
  
  if (argc < 2){
    fprintf(stderr, "[Error] The number of arguments must be two or over.");
    return 1;
  }
  user_input = argv[1];
  token = tokenize(user_input);
  

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  mov rax, %d\n", expect_number());

  while(token->kind != TK_EOF){
    if(consume('+')){
      printf("  add rax, %d\n", expect_number());
      continue;
    }
    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }
  printf("  ret\n");
  return 0;
}
