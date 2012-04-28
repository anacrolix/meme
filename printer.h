#pragma once

#include "lexer.h"

struct Printer {
    FILE *file;
    TokenType last;
};

void printer_init(Printer *, FILE *);
void printer_destroy(Printer *);
void print_atom(Printer *, char const *fmt, ...);
void print_token(Printer *, TokenType);

