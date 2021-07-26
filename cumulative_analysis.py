#!/usr/bin/env python3
import os
import re
import sys

import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import pandas as pd
import seaborn as sns


def obj_timeline(file_contents: bytes):
    objectives = []
    times = []
    for line in file_contents:
        if line.strip() == "==========":
            break
        match = re.match(r"objective\s=\s(\d+)", line)
        if match:
            objectives.append(int(match.group(1)))
            continue
        match = re.match(r"%\stime elapsed:\s(\d+\.\d+)\ss", line)
        if match:
            times.append(float(match.group(1)))
            continue

    assert len(objectives) > 0
    assert len(objectives) == len(times)

    return [(times[i], objectives[i]) for i in range(len(objectives))]


TAGMAP = {
    "original": "Base",
    "restart": "Restart Based LNS",
    "replay": "LNS Replay",
}

if __name__ == "__main__":
    folder = sys.argv[1]
    CONFIG = ["original", "restart"]
    solver = sys.argv[2]
    if solver == "Gecode":
        CONFIG.append("replay")
    statistics = dict()
    instances = set()
    # Read all the files
    for config in CONFIG:
        statistics[config] = dict()
        for root, dirs, files in os.walk(folder + "/" + config):
            for name in files:
                if not name.endswith(".sol"):
                    continue
                components = name[:-(4)].split(".")
                data = components[0]
                instances.add(data)
                seed = 1
                if len(components) > 1:
                    assert len(components) == 2
                    seed = components[1]

                if data not in statistics[config]:
                    statistics[config][data] = []

                with open(os.path.join(root, name)) as f:
                    contents = f.readlines()

                timeline = obj_timeline(contents)
                statistics[config][data].append(timeline)

    baseline = 0
    for data in instances:
        baseline += statistics["original"][data][0][0][1]

    times = []
    cumulative = []
    tag = []

    def emit(time, obj, conf):
        times.append(time)
        cumulative.append(obj)
        tag.append(TAGMAP[conf])

    for config in CONFIG:
        events = dict()
        for data in instances:
            length = len(statistics[config][data])
            for i in range(length):
                timeline = statistics[config][data][i]
                for j in range(1, len(timeline)):
                    if timeline[j][0] not in events:
                        events[timeline[j][0]] = 0
                    events[timeline[j][0]] += (
                        timeline[j][1] - timeline[j - 1][1]
                    ) / length

        sorted_events = sorted(events)
        incumbent = baseline
        emit(0, incumbent, config)
        for i in sorted_events:
            incumbent += events[i]
            emit(i, incumbent, config)
        emit(120, incumbent, config)

    df = pd.DataFrame(
        data={
            "Time (s)": times,
            "Cumulative Objective": cumulative,
            "Solver Version": tag,
        }
    )

    sns.set(font_scale=1.23, style="whitegrid", font="IBM Plex Sans")
    fig, ax = plt.subplots()
    ax.yaxis.set_major_formatter(ticker.EngFormatter())

    plot = sns.lineplot(
        data=df,
        x="Time (s)",
        y="Cumulative Objective",
        hue="Solver Version",
        style="Solver Version",
        linewidth=4,
    )
    plot.figure.savefig("output.pdf")
