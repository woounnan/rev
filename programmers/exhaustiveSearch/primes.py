from time import time
from itertools import permutations

def swap(s, src, dst):
    s = list(s)
    tp = s[src]
    s[src] = s[dst]
    s[dst] = tp
    return ''.join(s)
def permutation(s, start, end):
    global l
    for idx, ele in enumerate(s[start:], start=start):  
        '''
        #Permutation with repetition
        if start is not idx and s[start] is s[idx]:
            continue
        '''
        s = swap(s, start, idx)
        l += 1
        #print(s[:start+1])
        permutation(s, start+1, end)
        s = swap(s, start, idx)
def solution(numbers):
    permutation(numbers, 0, len(numbers)-1)


l = 0
numbers = 'aab'
start = time()
permutation(numbers, 0, len(numbers)-1)
print('[*] Running Time : ', time() - start, 's')
print('[*] length of permutations : ', l)

l = 0
start = time()
for i in range(1, len(numbers)+1):
    permute = list(permutations(numbers, i))
    l += len(permute)
print('[*] Running Time : ', time() - start, 's')
print('[*] length of permutations : ', l)
