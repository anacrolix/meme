#include <stdio.h>
#include <stdbool.h>

typedef enum {
    INVALID,
    ATOM,
    START,
    END,
    TT_EOF,
    QUOTE,
} TokenType;

typedef struct {
    int line, col;
    TokenType type;
    char *value;
} Token;

typedef struct Lexer {
    FILE *file;
    char const *file_name;
    int line, col;
    Token token[1];
} Lexer;

bool next_token(Lexer *lexer);
