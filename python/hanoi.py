#!/usr/bin/python
import sys

def hanoi(n, a, b, c):
    if not n == 0:
        hanoi(n-1, a, c, b)
        print("move %s->%s" % (a, c))
        hanoi(n-1, b, a, c)


if len(sys.argv) == 2:
    hanoi(int(sys.argv[1]), 'A', 'B', 'C')
