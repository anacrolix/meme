#pragma once

#include <stdbool.h>
#include <stdio.h>

typedef struct Printer {
    bool just_atom;
    FILE *file;
} Printer;
