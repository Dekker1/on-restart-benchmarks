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
import os
import re
import sys

from statistics import stdev


def compute_area(file):
    objectives = []
    times = [0]
    objectives.append(0)
    for line in file:
        # match = re.match(r'%\sinit_area\s=\s(\d+)', line)
        # match = re.match(r'%\sinit_area\s=\s(\d+)', line)
        # if match:
        #     objectives.append(int(match[1]))
        #     continue
        match = re.match(r"objective\s=\s(\d+)", line)
        if match:
            objectives.append(int(match.group(1)))
            continue
        match = re.match(r"%\stime elapsed:\s(\d+\.\d+)\ss", line)
        if match:
            times.append(float(match.group(1)))
            continue
        match = re.search(r"solveTime=(\d+(.\d+)?)", line)
        if match:
            times.append(float(match.group(1)))
            continue

    assert len(objectives) > 0
    assert len(objectives) + 1 == len(times)
    area = 0
    for i in range(len(objectives)):
        area += (times[i + 1] - times[i]) * objectives[i]
    return int(area)


folder = sys.argv[1]
statistics = dict()
instances = set()
for config in ["original", "restart", "replay"]:
    for root, dirs, files in os.walk(folder + "/" + config):
        for name in files:
            if name.endswith(".sol"):
                components = name[:-(4)].split(".")
                data = components[0]
                instances.add(data)
                seed = 1
                if len(components) > 1:
                    assert len(components) == 2
                    seed = components[1]

                if data not in statistics:
                    statistics[data] = dict()
                if config not in statistics[data]:
                    statistics[data][config] = []

                with open(os.path.join(root, name)) as f:
                    contents = f.readlines()
                # Area
                area = compute_area(contents)

                objective = "UNSAT"
                for line in contents[::-1]:
                    # Best objective
                    match = re.match(r"objective\s=\s(\d+)", line)
                    if match:
                        objective = int(match.group(1))
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
                    match = re.search(r"solveTime=(\d+(.\d+)?)", line)
                    if match:
                        solvetime = float(match.group(1))
                        continue
                    # Restarts
                    match = re.search(r"restarts=(\d+)", line)
                    if match:
                        restarts = int(match.group(1))
                        continue
                statistics[data][config].append(
                    (
                        area,
                        objective,
                        solvetime,
                        restarts,
                        nodes,
                    )
                )

for data in instances:
    for config in ["original", "restart", "replay"]:
        stats = statistics[data][config]
        cumulative = stats[0]
        for i in range(1, len(stats)):
            cumulative = tuple(map(sum, zip(cumulative, stats[i])))
        avg = tuple([x / len(stats) for x in cumulative])
        dev = stdev([x[1] for x in stats]) if len(stats) > 1 else 0
        # (avg area, avg objective, stdev objective)
        statistics[data][config] = (avg[0], avg[1], dev)

# Print header
print(
    """
\\begin{tabular}{l|rr|rr|rr}
\\toprule
& \multicolumn{2}{c|}{Gecode} & \multicolumn{2}{c|}{Gecode Restart} & \multicolumn{2}{c}{Gecode Replay}\\\\
Instance & $\intobj$ & $\minobj$ & $\intobj$ & $\minobj$ & $\intobj$ & $\minobj$ \\\\
\midrule
"""
)

sorted_instances = sorted(instances)
for data in sorted_instances:
    print(f"{data}", end="")
    for config in ["original", "restart", "replay"]:
        print(
            f" & {int(statistics[data][config][0] / 1000) }k & {int(statistics[data][config][1])}",
            end="",
        )
        if statistics[data][config][2] != 0:
            print("^{", end="")
            print(
                int(statistics[data][config][2] / statistics[data][config][1] * 100),
                # int(statistics[data][config][2]),
                end="",
            )
            print("}", end="")
    print(" \\\\")

# Print footer
print("\n\\bottomrule\n\end{tabular}")
