#include <stdio.h>
#include <stdbool.h>

typedef enum {
    INVALID,
    ATOM,
    START,
    END,
} TokenType;

typedef struct {
    int line, col;
    TokenType type;
    char *value;
} Token;

typedef struct {
    int line, col;
    char const *fname;
} Source;

typedef struct Lexer {
    FILE *file;
    int line, col;
    Token token[1];
} Lexer;

bool next_token(Lexer *lexer);
