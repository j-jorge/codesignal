#!/usr/bin/python3

import re
import sys

lines = {}
for line in open(sys.argv[1]):
    fields = re.split(r'[ \n/]+', line)
    alg = fields[0]
    n = int(fields[1])

    if alg not in lines:
        lines[alg] = {}

    lines[alg][n] = [float(fields[4]), int(fields[6])]

for k, d in lines.items():
    for n, f in d.items():
        if k != "best_ratings":
            f[0] = f[0] / lines["best_ratings"][n][0]
            f[1] = f[1] / lines["best_ratings"][n][1]

for n in lines["best_ratings"].keys():
    lines["best_ratings"][n] = (1, 1);

for k, d in lines.items():
    print("\"" + k.replace("_", "-") + "\"")
    for n, f in sorted(d.items()):
        print(n, f[0], f[1])
    print()
    print()
