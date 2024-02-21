import random
import math

WIDTH = 100
COUNT_INT = 355000
COUNT_MAX = COUNT_INT * 10000


hit = 0
count = 0
while count < COUNT_MAX:
    point = [random.randint(0, WIDTH - 1) + random.random(), random.randint(0, WIDTH - 1) + random.random()]
    distance = math.sqrt(math.pow(point[0], 2) + math.pow(point[1], 2))

    if distance < WIDTH:
        hit += 1

    count = count + 1;
    if count % COUNT_INT == 0:
        print(hit, count, hit / count * 4)

print("finally ", hit, count, hit / count * 4) 
