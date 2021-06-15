#!/usr/bin/env python3
"""
This script will produce the final value of a variable named 'objective' and its
area for every file ending in '.sol' in the directory provided. The files are
expected to be fzn-gecode output piped through solns2out with the
'--output-time' flag. Furthermore the file is expected to contain the initial
area which can be found by adding
'constraint trace("% init_area = \(ub(objective));\n", true);'
to the model in question.
"""
import csv
import os
import re
import sys

def compute_area(file):
    area = -1

    objectives = []
    times = [0]
    timeout = -1
    objectives.append(0)
    for line in contents:
        # match = re.match(r'%\sinit_area\s=\s(\d+)', line)
        # if match:
        #     objectives.append(int(match[1]))
        #     continue
        match = re.match(r'objective\s=\s(\d+)', line)
        if match:
            objectives.append(int(match[1]))
            continue
        match = re.match(r'%\stime elapsed:\s(\d+)\sms', line)
        if match:
            times.append(int(match[1]))
            continue
        match = re.search(r"solvetime:.*\((\d+).(\d+)\s+ms\)", line)
        if match:
            times.append(int(match.group(1)))
            continue

    assert(len(objectives) > 0)
    assert(len(objectives)+1 == len(times))
    area = 0
    for i in range(len(objectives)):
        area += ((times[i+1] - times[i])/1000)*objectives[i]
    return int(area)

folder = sys.argv[1]
statistics = {}
for root, dirs, files in os.walk(folder):
    for name in files:
        if name.endswith('.sol'):
            seed = 1
            match = re.search(r"\.(\d+)\.sol", name)
            if match:
                seed = int(match.group(1))
            with open(os.path.join(root, name)) as f:
                contents = f.readlines()
            # Area
            area = compute_area(contents)

            objective = 'UNSAT'
            for line in contents[::-1]:
                # Best objective
                match = re.match(r'objective\s=\s(\d+)', line)
                if match:
                    objective = int(match[1])
                    break

            nodes = -1
            solvetime = -1
            restarts = -1
            for line in contents:
                # Evaluation time
                match = re.search(r"copies:\s+(\d+)", line)
                if match:
                    nodes = int(match.group(1))
                    continue
                # Solve time
                match = re.search(r"solvetime:.*\((\d+).(\d+)\s+ms\)", line)
                if match:
                    solvetime = int(match.group(1))
                    continue
                # Restarts
                match = re.search(r"restarts:\s+(\d+)", line)
                if match:
                    restarts = int(match.group(1))
                    continue
            statistics[name[:-(4)].replace(".", ",")] = (area, objective, solvetime, restarts, nodes)

sorted_stats = sorted(statistics.items())
a = sorted_stats[0][0][:sorted_stats[0][0].find(",")]
for key, val in sorted_stats:
    if key[:key.find(",")] != a:
        print("\n\n")
        a = key[:key.find(",")]
    print("%s,%s" % (key, ",".join([v.__str__() for v in val])))

exit(1)
