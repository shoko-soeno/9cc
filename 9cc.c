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

typedef struct Token Token;

// token型
struct Token {
  TokenKind kind; //トークンの型
  Token *next; //次の入力トークン
  int val; //kindがTK_NUMの場合、その数値
  char *str; //token文字列
}

Token *token;

void error(char *fmt, )

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  char *p = argv[1];

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  printf("  mov rax, %ld\n", strtol(p, &p, 10)); //raxはレジスタの一つ

  while (*p)   
  {
    if (*p == '+') {
      p++;
      printf("  add rax, %ld\n", strtol(p, &p, 10));
      continue;
    }

    if (*p == '-') {
      p++;
      printf("  sub rax, %ld\n", strtol(p, &p, 10));
      continue;
    }

    fprintf(stderr, "予期しない文字です: '%c'\n", *p);
    return 1;
  }

  printf("  ret\n");
  return 0;
}

