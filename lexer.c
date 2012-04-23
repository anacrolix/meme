#include "lexer.h"
#include <stdlib.h>
#include <string.h>

char *read_atom(Lexer *lexer) {
    // TODO remove atom length limit
    char lval[0x100];
    size_t len = 0;
    for (; len < sizeof lval - 1; len++) {
        int c = fgetc(lexer->file);
        if (c < 0) break;
        switch (c) {
        case ' ':
        case '\t':
        case '(':
        case ')':
        case '\r':
        case '\n':
            if (c != ungetc(c, lexer->file)) abort();
            goto end_loop;
        }
        lval[len] = c;
        lexer->col++;
    }
end_loop:
    if (!len) abort();
    lval[len] = '\0';
    return strdup(lval);
}

__attribute__((warn_unused_result))
static bool discard_whitespace(Lexer *lexer) {
    for (;;) {
        int c = fgetc(lexer->file);
        if (c < 0) return false;
        switch (c) {
        case ' ':
        case '\t':
            lexer->col++;
            break;
        case '\n':
            lexer->line++;
            lexer->col = 1;
            break;
        default:
            if (c != ungetc(c, lexer->file)) abort();
            return true;
        }
    }
}

void init_token(Lexer *lexer, TokenType type) {
    Token *t = lexer->token;
    t->type = type;
    t->line = lexer->line;
    t->col = lexer->col;
}

bool next_token(Lexer *lexer) {
    if (!discard_whitespace(lexer)) {
        init_token(lexer, INVALID);
        return false;
    }
    int c = fgetc(lexer->file);
    if (c < 0) {
        init_token(lexer, INVALID);
        return false;
    }
    switch (c) {
    case '(':
        init_token(lexer, START);
        break;
    case ')':
        init_token(lexer, END);
        break;
    case '\'':
        init_token(lexer, QUOTE);
        break;
    default:
        if (c != ungetc(c, lexer->file)) abort();
        init_token(lexer, ATOM);
        free(lexer->token->value);
        lexer->token->value = read_atom(lexer);
        return !!lexer->token->value;
    }
    lexer->col++;
    return true;
}

