#pragma once

#include "types.h"
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

struct Token {
    int line, col;
    TokenType type;
    char *value;
};

struct Lexer {
    FILE *file;
    char const *file_name;
    int line, col;
    Token token[1];
};

bool next_token(Lexer *lexer);
