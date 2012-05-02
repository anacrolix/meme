import functools
import sys

sys.setrecursionlimit(100000)

#@functools.lru_cache(maxsize=None)
def ack(m, n):
    if m:
        if n:
            return ack(m - 1, ack(m, n - 1))
        else:
            return ack(m - 1, 1)
    else:
        return n + 1

print(ack(int(sys.argv[1]), int(sys.argv[2])))
