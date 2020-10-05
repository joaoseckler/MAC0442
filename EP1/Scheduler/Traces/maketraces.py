#!/usr/bin/env python3

from random import randrange, uniform
import math

for N in 10,100,1000:
    l = []
    for i in range(N):
        t0 = randrange(0, N)
        dt = randrange(1, 20)
        deadline = t0 + dt + int(dt * uniform(0.05, 2))
        l.append((t0, dt, deadline))

    l.sort()
    with open(f"trace{int(math.log(N, 10)+0.1)}.txt", "w") as f:
        for i in range(N):
            l[i] = f"P{i + 1} {l[i][0]} {l[i][1]} {l[i][2]}"
            f.write(l[i])
            f.write("\n")



