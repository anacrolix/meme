#include "meme.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

static Node *run_file(FILE *file, Env *env) {
    Lexer lexer = {
        .file = file,
        .line = 1,
        .col = 1,
    };
    next_token(&lexer);
    Node *result = NULL;
    while (lexer.token->type != INVALID) {
        if (result) node_unref(result);
        Node *node = parse(&lexer);
        result = eval(node, env);
        node_unref(node);
        if (!result) break;
    }
    free(lexer.token->value);
    return result;
}

int main(int argc, char **argv) {
    Node *result = NULL;
    Env *env = top_env_new();
    if (argc == 1) {
        result = run_file(stdin, env);
    } else {
        for (int i = 1; i < argc; i++) {
            FILE *file = fopen(argv[i], "rb");
            if (!file) {
                fprintf(stderr, "%s: %s\n", argv[i], strerror(errno));
                if (result) node_unref(result);
                result = NULL;
                break;
            }
            if (result) node_unref(result);
            result = run_file(file, env);
            if (fclose(file)) abort();
            if (!result) break;
        }
    }
    node_unref(env);
    if (result) {
        node_print(result, &(Printer){.file=stdout});
        putchar('\n');
        node_unref(result);
        return 0;
    }
    return 1;
}
