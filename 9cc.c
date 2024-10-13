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

    if (*p == '+' || *p == '-')
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

    error_at(p, "expected a number");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}
// long int strtol(const char *nptr, char **endptr, int base);
// strtol(p, &p, 10) 
// p: 数値部分が始まる文字列の位置。
// &p: 数値の変換後、p は自動的に数値の次の位置まで進む（数値の部分をスキップ）
// 10: 10進数として数値を解析


int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズする
  user_input = argv[1];
  token = tokenize();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 式の最初は数でなければならないのでそれをチェック
  // 最初の数をレジスタにロード
  printf("  mov rax, %d\n", expect_number()); //raxはレジスタの一つ

  // '+ <数>' あるいは '- <数>' というトークンの並びを消費しつつ
  // アセンブリを出力
  while (!at_eof())
  {
    if (consume('+'))
    {
      printf("  add rax, %d\n", expect_number());
      continue;
    }
    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }
  printf("  ret\n");
  return 0;
}

