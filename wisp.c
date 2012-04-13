#include <glib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    ATOM,
    START,
    END,
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

typedef struct {
    GSList *indents;
    FILE *file;
    int cur_indent;
    bool new_line;
} Tokenizer;

void read_atom(Tokenizer *toker, char **value) {
    char lval[0x100];
    int len = 0;
    for (; len < sizeof lval - 1; len++) {
        int c = fgetc(toker->file);
        if (c < 0) break;
        switch (c) {
        case ' ':
        case '(':
        case ')':
        case '\n':
            if (c != ungetc(c, toker->file)) abort();
            goto end_loop;
        }
        lval[len] = c;
    }
end_loop:
    if (!len) abort();
    lval[len] = '\0';
    *value = strdup(lval);
}

bool next_token(Tokenizer *toker, Token *token) {
    for (;;) {
        int c = fgetc(toker->file);
        if (c < 0) {
            if (ferror(toker->file)) abort();
            if (!toker->indents) return false;
            toker->indents = g_slist_remove_link(toker->indents, toker->indents);
            token->type = END;
            return true;
        }
        switch (c) {
        case ' ':
            break;
        case '\n':
            toker->cur_indent = 0;
            toker->new_line = true;
            continue;
        case '(':
        case ')':
            *token = (Token){
                .type = (c == '(') ? START : END,
            };
            toker->cur_indent++;
            return true;
        default:
            if (c != ungetc(c, toker->file)) abort();
            if (toker->new_line) {
                if (toker->indents && toker->cur_indent <= GPOINTER_TO_INT(toker->indents->data)) {
                    toker->indents = g_slist_remove_link(toker->indents, toker->indents);

                    token->type = END;
                    return true;
                }
                toker->new_line = false;
                token->type = START;
                toker->indents = g_slist_prepend(toker->indents, GINT_TO_POINTER(toker->cur_indent));
                return true;
            }
            read_atom(toker, &token->value);
            token->type = ATOM;
            return true;
        }
        toker->cur_indent++;
    }
}

typedef enum {
    VARREF,
    DOUBLE,
    INTEGER,
    STRING,
} Form;

typedef struct {
    Form form;
    union {
        long long i;
        double f;
        char *s;
        GSList *list;
    };
} Node;

Node *parse(Tokenizer *tr) {
    Token tn[1];
    if (!next_token(tr, tn)) return NULL;
    Node *node = malloc(sizeof *node);
    switch (tn->type) {
    case ATOM:
        if (tn->value[0] == '"') {
            node->form = STRING;
            node->s = strdup(tn->value + 1);
            return node;
        } else if (strchr(tn->value, '.')) {
            node->form = DOUBLE;
            if (1 != sscanf(tn->value, "%lf", &node->f)) abort();
        } else if (isdigit(tn->value)) {
            node->form = INTEGER;
            if (1 != sscanf(tn->value, "%lld", &node->i)) abort();
        } else {
            node->form = VARREF;
            node->s = strdup(tn->value);
        }
    case START:

    }




}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s script\n", argv[0]);
        return 2;
    }
    FILE *token_file = fopen(argv[1], "rb");
    if (!token_file) {
        perror("fopen");
        return 1;
    }
    Tokenizer toker = {
        .file = token_file,
        //.indents = g_slist_prepend(NULL, GINT_TO_POINTER(-1)),
        .new_line = true,
    };
    for (;;) {
        Token token[1];
        if (!next_token(&toker, token)) break;
        switch (token->type) {
        case ATOM:
            printf("%s\n", token->value);
            free(token->value);
            break;
        case START:
            puts("(");
            break;
        case END:
            puts(")");
            break;
        default:
            abort();
        }
    }
}
