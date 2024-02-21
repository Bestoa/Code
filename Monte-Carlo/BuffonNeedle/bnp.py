import random
import math

WIDTH = 100
L = 1
T = 10
COUNT_INT = 355000
COUNT_MAX = COUNT_INT * 100


hit = 0
count = 0
while count < COUNT_MAX:
    start = random.randint(0, WIDTH - 1) + random.random()
    degree = random.randint(0, 359) + random.random()
    radian = math.radians(degree)
    end = start + math.sin(radian) * L

    line1 = math.floor(start / T) * T
    line2 = line1 + T

    if (end >= start and end >= line2) or (end <= start and end < line1):
        hit = hit + 1;

    count = count + 1;
    if count % 35500 == 0:
        print(hit, count, 2 * count * L / (hit * T))

print("finally ", hit, count, 2 * count * L / (hit * T))
