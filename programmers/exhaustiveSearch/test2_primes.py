primeSet = set()
def makeCombinations(str1, str2):
    if str1 != "":
        input('result : ' + str1)

    for i in range(len(str2)):
        print('i=', i)
        input(str2[:i])
        input(str2[i+1:])
        makeCombinations(str1 + str2[i], str2[:i] + str2[i + 1:])
        print('====================')


def solution(numbers):
    makeCombinations("", numbers)

    answer = len(primeSet)

    return answer

solution('17')
