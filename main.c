#include "meme.h"
#include <stdio.h>

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
    Lexer lexer = {
        .file = token_file,
        .line = 1,
        .col = 1,
    };
    Env *top_env = top_env_new();
    next_token(&lexer);
    Node *result = NULL;
    while (lexer.token->type != INVALID) {
        if (result) node_unref(result);
        Node *node = parse(&lexer);
        print_node(node, &(Printer){.file=stderr});
        putchar('\n');
        result = eval(node, top_env);
        node_unref(node);
        print_node(result, &(Printer){.file=stderr});
        putchar('\n');
    }
    node_unref(top_env);
    if (fclose(token_file)) abort();
    return !result;
}
