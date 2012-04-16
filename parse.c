#include "parse.h"
#include "meme.h"
#include <ctype.h>
#include <string.h>

Node *parse_list(Lexer *lexer) {
    Token *const t = lexer->token;
    List *ret = list_new();
    while (t->type != END) {
        Node *node = parse(lexer);
        if (!node) abort();
        ret = list_append(ret, node);
    }
    next_token(lexer);
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
        ret = (Node *)var_new(t->value);
    }
    next_token(lexer);
    return ret;
}

Node *parse(Lexer *lexer) {
    switch (lexer->token->type) {
    case ATOM:
        return parse_atom(lexer);
    case START:
        if (!next_token(lexer)) abort();
        return parse_list(lexer);
    default:
        abort();
    }
}

