#include "meme.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

static Node *run_file(FILE *file, Env *env, char const *name) {
    Lexer lexer = {
        .file = file,
        .line = 1,
        .col = 1,
        .file_name = name,
    };
    Node *result = NULL;
    for (;;) {
        if (!next_token(&lexer)) break;
        if (result) node_unref(result);
        result = NULL;
        Node *node = parse(&lexer);
        if (!node) break;
        result = node_eval(node, env);
        node_unref(node);
        // this is here to stress test the cycle collector
        collect_cycles();
        if (!result && !isatty(fileno(file))) break;
        if (result && !void_check(result)) {
            node_print_file(result, stdout);
            putchar('\n');
        }
    }
    free(lexer.token->value);
    return result;
}

int main(int argc, char **argv) {
    meme_init();
    Node *result = NULL;
    Env *env = top_env_new();
    if (argc == 1) {
        result = run_file(stdin, env, "<stdin>");
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
            result = run_file(file, env, argv[i]);
            if (fclose(file)) abort();
            if (!result) break;
        }
    }
    collect_cycles();
    node_unref((Node *)env);
    int ret = !result;
    collect_cycles();
    if (result) node_unref(result);
    meme_final();
    return ret;
}

