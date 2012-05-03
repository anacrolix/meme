#pragma once

struct Quote {
    Node;
    Node *quoted;
};

extern Type const quote_type;

Quote *quote_new(Node *);

