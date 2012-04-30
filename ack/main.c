#include <stdio.h>

typedef long long Int;

Int ackermann_peter(Int m, Int n) {
    if (m) {
        if (n) return ackermann_peter(m - 1, ackermann_peter(m, n - 1));
        else return ackermann_peter(m - 1, 1);
    } else return n + 1;
}

int main(int argc, char **argv) {
    printf("%lld\n", ackermann_peter(atoll(argv[1]), atoll(argv[2])));
    return 0;
}
