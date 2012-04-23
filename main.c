#include "meme.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static Node *run_file(FILE *file, Env *env) {
    Lexer lexer = {
        .file = file,
        .line = 1,
        .col = 1,
    };
    Node *result = NULL;
    for (;;) {
        if (!next_token(&lexer)) break;
        if (result) node_unref(result);
        Node *node = parse(&lexer);
        result = eval(node, env);
        node_unref(node);
        if (!result) break;
        if (!void_check(result)) {
            node_print(result, &(Printer){.file=stdout});
            putchar('\n');
        }
        // this is here to stress test the cycle collector
        collect_cycles();
    }
    free(lexer.token->value);
    return result;
}

int main(int argc, char **argv) {
    meme_init();
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
    collect_cycles();
    node_unref(env);
    int ret = !result;
    collect_cycles();
    if (result) node_unref(result);
    meme_final();
    return ret;
}

