#include "parse.h"
#include "meme.h"
#include <ctype.h>
#include <string.h>

Pair *parse_list(Lexer *lexer) {
    Token *const t = lexer->token;
    if (!next_token(lexer)) {
        fprintf(stderr, "unterminated list (%s:%d:%d)\n", 
                lexer->file_name, lexer->line, lexer->col);
        return NULL;
    }
    if (t->type == END) {
        node_ref(nil_node);
        return nil_node;
    }
    Node *addr = parse(lexer);
    if (!addr) return NULL;
    Pair *dec = parse_list(lexer);
    if (!dec) {
        node_unref(addr);
        return NULL;
    }
    Pair *ret = pair_new();
    ret->addr = addr;
    ret->dec = dec;
    return ret;
}

Node *parse_atom(Lexer *lexer) {
    Token *const t = lexer->token;
    Node *ret = NULL;
    if (isdigit(*t->value)) {
        // TODO how do i do typeof(IntNode.i) again? ffs
        long long i;
        if (1 != sscanf(t->value, "%lld", &i)) abort();
        ret = int_new(i);
    } else if (!strcmp(t->value, "#t")) {
        node_ref(true_node);
        ret = true_node;
    } else if (!strcmp(t->value, "#f")) {
        node_ref(false_node);
        ret = false_node;
    } else {
        ret = (Node *)symbol_new(t->value);
    }
    return ret;
}

static Node *parse_quote(Lexer *lexer) {
    next_token(lexer);
    Node *quoted = parse(lexer);
    if (!quoted) return NULL;
    Quote *ret = malloc(sizeof *ret);
    node_init(ret, &quote_type);
    ret->quoted = quoted;
    return ret;
}

Node *parse(Lexer *lexer) {
    switch (lexer->token->type) {
    case ATOM:
        return parse_atom(lexer);
    case START:
        return parse_list(lexer);
    case QUOTE:
        return parse_quote(lexer);
    default:
        fprintf(stderr, "syntax error at line %d col %d\n", lexer->token->line, lexer->token->col);
        return NULL;
    }
}

