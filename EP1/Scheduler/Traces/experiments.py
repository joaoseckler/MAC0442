#!/usr/bin/env python3

import pandas as pd
import subprocess
import statistics as st

REPS = 30
TRACES = ["trace1.txt", "trace2.txt", "trace3.txt"]

deadline = []
context = []

results = {}

for trace in TRACES:
    for alg in [1, 2, 3]:
        for rep in range(REPS):
            execute = subprocess.Popen(["../ep1", alg, f"out_{trace}_{alg}_{rep}.txt"],
                                       shell=True,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE,
                                       universal_newlines=True)

            with open("deadlines", 'r') as file:
                deadline.append(float(file.readline()))

            with open(f"out_{trace}_{alg}_{rep}.txt", 'r') as file:
                context.append(int(file.readlines()[-1]))

        mean_deadline = st.mean(deadline)
        mean_context = st.mean(context)
        ci_deadline = 1.96 * st.stdev(deadline)
        ci_context = 1.96 * st.stdev(context)

        results[f"{trace}_{alg}"] = (mean_deadline, ci_deadline,
                                     mean_context, ci_context)

        deadline = []
        context = []

pd_results = pd.DataFrame(data=results)
pd_results.to_csv("results.csv", index=False, header=True)
