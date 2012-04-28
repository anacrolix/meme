#include "printer.h"
#include <stdlib.h>
#include <stdarg.h>

void printer_init(Printer *p, FILE *f) {
    *p = (Printer){
        .file = f,
    };
}

void printer_destroy(Printer *p) {
}

static void print_space_if_required(Printer *p, TokenType tt) {
    switch (tt) {
    case INVALID:
    case TT_EOF:
        abort();
    case ATOM:
    case START:
    case QUOTE:
        switch (p->last) {
        case START:
        case QUOTE:
        case INVALID:
            return;
        default:
            break;
        }
        break;
    case END:
        switch (p->last) {
        case ATOM:
        case END:
        case START:
            return;
        default:
            abort();
        }
    }
    fputc(' ', p->file);
}

void print_atom(Printer *p, char const *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    print_space_if_required(p, ATOM);
    vfprintf(p->file, fmt, ap);
    va_end(ap);
    p->last = ATOM;
}

void print_token(Printer *p, TokenType tt) {
    print_space_if_required(p, tt);
    switch (tt) {
    case START:
        fputc('(', p->file);
        break;
    case END:
        fputc(')', p->file);
        break;
    case QUOTE:
        fputc('\'', p->file);
        break;
    default:
        abort();
    }
    p->last = tt;
}

