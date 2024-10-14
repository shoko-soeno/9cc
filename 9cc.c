#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //isspace, isdigit
#include <stdarg.h> //va_list, vprintf
#include <string.h>
#include <stdbool.h>

// tokenの種類
typedef enum {
  TK_RESERVED, //記号
  TK_NUM, //整数トークン
  TK_EOF, //入力の終わりを表すトークン
} TokenKind;

// 連結リスト生成のため、自己参照構造体を定義
typedef struct Token Token;

// token型
struct Token {
  TokenKind kind; //トークンの型
  Token *next; //次の入力トークン
  int val; //kindがTK_NUMの場合、その数値
  char *str; //token文字列
};

// input program
char *user_input;

// 現在着目しているトークン
Token *token;

// report error and exit
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// report error location and exit
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号の場合、トークンを1つ読み進めて真を返す
// それ以外の場合、偽を返す
bool consume(char op)
{
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号の場合、トークンを1つ読み進める
// それ以外の場合、errorを出力
void expect(char op)
{
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "expected '%c'", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
// それ以外の場合、errorを出力
int expect_number()
{
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}

// 新しいトークンを作成し、curに繋げる
// TokenKind kind : トークンの種類
// Token *cur : 連結リストの現在のトークン(リストの末尾)
// char *str : 新しいトークン文字列(例えば数値や記号)
Token *new_token(TokenKind kind, Token *cur, char *str)
{
  Token *tok = calloc(1, sizeof(Token)); //create new token
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok; //return new token
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize()
{
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    //skip whitespace
    if (isspace(*p))
    {
      p++;
      continue;
    }

    // Punctuator
    if (strchr("+-*/()", *p))
    {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10); //変換後の位置を endptr で取得できる
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}
// long int strtol(const char *nptr, char **endptr, int base);
// strtol(p, &p, 10) 
// p: 数値部分が始まる文字列の位置。
// &p: 数値の変換後、p は自動的に数値の次の位置まで進む（数値の部分をスキップ）
// 10: 10進数として数値を解析


// Parser
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // integer
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node {
  NodeKind kind;
  Node *lhs; // left hand side
  Node *rhs; // right hand side
  int val; // kindがND_NUMの場合のみ使う
};

Node *new_node(NodeKind kind)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val)
{
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr();
Node *mul();
Node *primary();

// expr = mul ("+" mul | "-" mul)*
// *は何回も繰り返すことができる、つまり、+や-を見つける度に処理をする。
// 左結合（木の左から順番に計算）
Node *expr()
{
  Node *node = mul();

  for (;;)
  {
    if (consume('+'))
      node = new_binary(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = primary ("*" primary | "/" primary)*
Node *mul()
{
  Node *node = primary(); // primaryは数値やカッコで囲まれた式を処理

  for (;;)
  {
    if (consume('*')) // 左辺として既に処理した部分式node、右辺としてprimary()で得られる部分式を新しいノードとして返す
      node = new_binary(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_binary(ND_DIV, node, primary());
    else
      return node;
  }
}

// primary = num | "(" expr ")"
Node *primary()
{
  if (consume('('))
  {
    Node *node = expr();
    expect(')');
    return node;
  }

  return new_num(expect_number());
}

// code generator
void gen(Node *node)
{
  if (node->kind == ND_NUM)
  {
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
  default:
    error("invalid node kind");
  }

  printf("  push rax\n");
}


int main(int argc, char **argv) {
  if (argc != 2)
    error("引数の個数が正しくありません\n", argv[0]);

  // トークナイズする
  user_input = argv[1];
  token = tokenize();
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // // 式の最初は数でなければならないのでそれをチェック
  // // 最初の数をレジスタにロード
  // printf("  mov rax, %d\n", expect_number()); //raxはレジスタの一つ

  // // '+ <数>' あるいは '- <数>' というトークンの並びを消費しつつ
  // // アセンブリを出力
  // while (!at_eof())
  // {
  //   if (consume('+'))
  //   {
  //     printf("  add rax, %d\n", expect_number());
  //     continue;
  //   }
  //   expect('-');
  //   printf("  sub rax, %d\n", expect_number());
  // }

  // Traverse the AST to emit assembly
  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}


