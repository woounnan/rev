from time import time
from itertools import permutations
from collections import Counter

def solution(numbers):
    _max = int(''.join(sorted(numbers, reverse=True)))
    check = [True] * _max
    primes = []

    for i in range(2, _max+1):
        if check[i-1] is True:
           primes.append(str(i)) 
           for n in range(i, _max+1, i):
               check[n-1] = False

    c_n = Counter(numbers) 

    l = 0
    for prime in primes:
        c_p = Counter(prime)
        c = 0
        for k in c_p.keys():
            if c_n[k] and (c_n[k] - c_p[k]) >= 0:
                c += 1        
        if c == len(c_p.keys()):
            l += 1
    return l

numbers = '011'
solution(numbers)
