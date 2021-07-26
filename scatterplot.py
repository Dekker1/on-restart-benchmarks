#!/usr/bin/env python3
import sys

import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

if __name__ == "__main__":
    data = pd.read_csv(sys.argv[1])

    sns.set(font_scale=1.24, style="whitegrid", font="IBM Plex Sans")
    fig, ax = plt.subplots()

    plot = sns.scatterplot(
        data=data,
        x="Compile Time (s)",
        y="Solve Time (s)",
        hue="Configuration",
        style="Configuration",
        legend="gbac" in sys.argv[1],
    )

    if "radiation" in sys.argv[1]:
        ax.set_xlim(0, 0.5)
        ax.set_ylim(0, 0.5)
    elif "gbac" in sys.argv[1]:
        handles, labels = ax.get_legend_handles_labels()
        ax.legend(handles=handles, labels=labels)
        plot.legend(bbox_to_anchor=(0.23, 0.95), loc="upper left", borderaxespad=0)
        ax.set_xlim(0, 30)
        ax.set_ylim(0, 30)

    plot.figure.savefig("output.pdf", bbox_inches="tight")
