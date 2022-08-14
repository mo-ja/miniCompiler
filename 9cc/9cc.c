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

bool consume(char *op){
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(op, token->str, token->len))
    return false;

  token = token->next;
  return true;
}

void expect(char *op){
  if (token->kind != TK_RESERVED ||
      token->len != strlen(op) ||
      memcmp(op, token->str, token->len))
    error_at(token->str, "%s is expected but not detected.", op);
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
    if( !memcmp(p, "==", 2)||
        !memcmp(p, "!=", 2)||
        !memcmp(p, "<=", 2)||
        !memcmp(p, ">=", 2)){
      cur = new_token(TK_RESERVED, cur, p);
      p+=2;
      cur->len = 2;
      continue;
    }
    if(*p == '+' || *p == '-' || *p == '(' || *p == ')' || *p == '*' || *p == '/' || *p == '<' || *p == '>'){
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
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

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *expr();
Node *primary();
Node *unary();
Node *equality();
Node *relational();
Node *add();
Node *mul();

Node *expr(){
  return equality();
}

Node *equality(){
  Node *node = relational();
  for(;;){
    if(consume("==")){
      node = new_node(ND_EQ, node, relational());
    }else if(consume("!=")){
      node = new_node(ND_NE, node, relational());
    }else{
      return node;
    }
  }
}

Node *relational(){
  Node *node = add();
  for(;;){
    if(consume("<")){
      node = new_node(ND_LT, node, add());
    }else if(consume("<=")){
      node = new_node(ND_LE, node, add());
    }else if(consume(">")){
      node = new_node(ND_LT, add(), node);
    }else if(consume(">=")){
      node = new_node(ND_LE, add(), node);
    }else{
      return node;
    }
  }
}

Node *add(){
  Node *node = mul();
  for(;;){
    if(consume("+")){
      node = new_node(ND_ADD, node, mul());
    }else if(consume("-")){
      node = new_node(ND_SUB, node, mul());
    }else{
      return node;
    }
  }
}

Node *mul(){
  Node *node = unary();
  for(;;){
    if(consume("*")){
      node = new_node(ND_MUL, node, unary());
    }else if(consume("/")){
      node = new_node(ND_DIV, node, unary());
    }else{
      return node;
    }
  }
}

Node *primary(){
  if (consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }else{
    int num = expect_number();
    Node *node = new_node_num(num);
    return node;
  }
}

Node *unary(){
  if(consume("-")){
    return new_node(ND_SUB, new_node_num(0), primary());
  }else if (consume("+")){
    return primary();
  }else{
    return primary();
  }
}

void gen(Node *node){
  if (node->kind == ND_NUM){
    printf("  push %d\n", node->val);
    return;    
  }

  gen(node->lhs);
  gen(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind)
  {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  default:
    break;
  }

  printf("  push rax\n");
}

int main(int argc, char** argv)
{
  
  if (argc < 2){
    fprintf(stderr, "[Error] The number of arguments must be two or over.");
    return 1;
  }
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
