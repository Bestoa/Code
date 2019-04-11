#!/usr/bin/python
# Slow version. For fast version with Cython, please refer to https://github.com/Bestoa/PyPrime
import sys

prime_bitmap = None
_bit_num = 32

def GETB(x, b):
    return (x) & (1 << b)

def SETB(x, b):
    return (x) | (1 << b)

def RESETB(x, b):
    return (x) & (~(1 << b))

class Prime:
    def __init__(self, end):
        self.bitmap = list(0 for _ in range(end // _bit_num + 1))
        print('Init bitmap done')
        self.count = 0
        self.end = end

    def _isPrime(self, n):
        index1 = n // _bit_num
        index2 = n % _bit_num
        return not GETB(self.bitmap[index1], index2)

    def _setNotPrime(self, n):
        index1 = n // _bit_num
        index2 = n % _bit_num
        self.bitmap[index1] = SETB(self.bitmap[index1], index2)

    def cal(self):
        self._setNotPrime(0)
        self._setNotPrime(1)
        i = 2;
        while i < self.end + 1:
            if self._isPrime(i):
                self.count += 1
                j = 2 * i
                while j < self.end + 1:
                    self._setNotPrime(j)
                    j += i
            i += 1
        print('There are %d prime numbers in [0, %d]' % (self.count, self.end))

if len(sys.argv) == 2:
    p = Prime(int(sys.argv[1]))
    p.cal()
else:
    print('Usage: python prime.py number')
