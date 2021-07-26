#!/usr/bin/env python3
import os
import sys
import time
import csv

import logging

logging.basicConfig(filename="minizinc-python.log", level=logging.DEBUG)

MZNR_HOME = os.getcwd() + "/software/install/minizinc/bin"
os.environ["PATH"] = MZNR_HOME + ":" + os.environ["PATH"]
os.environ["MZN_SOLVER_PATH"] = (
    os.getcwd() + "/software/install/gecode/share/minizinc/solvers"
)
import minizinc

MZA_HOME = os.getcwd() + "/software/mza"
sys.path.append(MZA_HOME)

mza_lib = os.getcwd() + "/software/install/mza/lib"
if sys.platform == "linux" or sys.platform == "linux2":
    rerun = True
    if not "LD_LIBRARY_PATH" in os.environ:
        os.environ["LD_LIBRARY_PATH"] = mza_lib
    elif not mza_lib in os.environ.get("LD_LIBRARY_PATH"):
        os.environ["LD_LIBRARY_PATH"] += ":" + mza_lib
    else:
        rerun = False
    if rerun:
        os.execve(os.path.realpath(__file__), sys.argv, os.environ)

import mza
from mza import Instance

mza.DEBUG = False

SOLVER = "gecode_presolver"
PROTO_MODEL = "radiation/proto.uzn"
RESTART_MODEL = "radiation/on_restart.mzn"
DATA = [
    "radiation/01.dzn",
    "radiation/02.dzn",
    "radiation/03.dzn",
    "radiation/04.dzn",
    "radiation/05.dzn",
    "radiation/06.dzn",
    "radiation/07.dzn",
    "radiation/08.dzn",
    "radiation/09.dzn",
]
FN_ID = "f_lex_obj_i"
N_OBJ = 2

RUNS = 10


def radiation_restart(data_file):
    os.environ["MZN_STDLIB_DIR"] = (
        os.getcwd() + "/software/install/minizinc/share/minizinc"
    )
    gecode = minizinc.Solver.lookup("gecode")
    inst = minizinc.Instance(gecode, minizinc.Model(RESTART_MODEL))
    inst.add_file(data_file)

    args = {
        "all_solutions": True,
        "--restart": "constant",
        "--restart-scale": 100000000,
    }
    res = inst.solve(**args)

    # print(res.statistics)
    return (
        res.statistics["flatTime"].total_seconds(),
        (res.statistics["initTime"] + res.statistics["solveTime"]).total_seconds(),
    )


def radiation_incr(data_file):
    os.environ["MZN_STDLIB_DIR"] = os.getcwd() + "/software/mza/share/minizinc"
    compile_time = 0.0
    solve_time = 0.0

    # --- Initial compilation of instance ---
    start = time.perf_counter()
    inst = Instance(PROTO_MODEL, data_file, SOLVER)
    inst.output_dict(False)
    compile_time += time.perf_counter() - start
    # --- Solve initial instance ---
    start = time.perf_counter()
    (status, sol) = inst.solve()
    solve_time += time.perf_counter() - start
    # print(status + ": " + str(sol))

    # --- Further Lexicographic Search ---
    stage = 1
    while stage <= N_OBJ:
        with inst.branch() as child:
            # --- Compile instance ---
            start = time.perf_counter()
            inst.add_call(FN_ID, stage)
            compile_time += time.perf_counter() - start
            # inst.print()

            # --- Solve instance ---
            start = time.perf_counter()
            (status, sol) = inst.solve()
            solve_time += time.perf_counter() - start
            if status == "UNSAT":
                stage += 1
            else:
                assert status == "SAT" or status == "OPT"
            # print(status + ": " + str(sol))

    return compile_time, solve_time


def radiation_redo(data_file):
    os.environ["MZN_STDLIB_DIR"] = os.getcwd() + "/software/mza/share/minizinc"
    compile_time = 0.0
    solve_time = 0.0

    incumbent = None
    stage = 1
    status = None
    inst = None
    while stage <= N_OBJ:
        # --- Compile instance ---
        start = time.perf_counter()

        inst = Instance(PROTO_MODEL, data_file, SOLVER)
        inst.output_dict(True)
        if incumbent is not None:
            inst.set_incumbent(incumbent)
            inst.add_call(FN_ID, stage)
        compile_time += time.perf_counter() - start

        # --- Solve instance ---
        start = time.perf_counter()
        (status, sol) = inst.solve()
        solve_time += time.perf_counter() - start
        if status == "UNSAT":
            stage += 1
        else:
            assert status == "SAT" or status == "OPT"
            incumbent = sol
        # print(
        #     status + ": [" + ", ".join([str(v) for k, v in incumbent.items()]) + "]"
        # )

    return compile_time, solve_time


if __name__ == "__main__":
    fieldnames = ["Configuration", "Data", "Compile Time (s)", "Solve Time (s)"]
    writer = csv.writer(sys.stdout)

    writer.writerow(fieldnames)

    for d in DATA:
        # --- Run Restart based strategy
        t1, t2 = 0, 0
        for i in range(RUNS):
            ct, st = radiation_restart(d)
            t1 += ct
            t2 += st
        writer.writerow(["RBS", d, t1 / RUNS, t2 / RUNS])
        # --- Run incremental rewriting
        t1, t2 = 0, 0
        for i in range(RUNS):
            ct, st = radiation_incr(d)
            t1 += ct
            t2 += st
        writer.writerow(["Incr.", d, t1 / RUNS, t2 / RUNS])
        # --- Run baseline
        t1, t2 = 0, 0
        for i in range(RUNS):
            ct, st = radiation_redo(d)
            t1 += ct
            t2 += st
        writer.writerow(["Base", d, t1 / RUNS, t2 / RUNS])

# df = pd.DataFrame(
#     data={
#         "Compile Time (s)": compile_time,
#         "Solve Time (s)": solve_time,
#         "Data": cumulative,
#         "Strategy": tag,
#     }
# )

# plot = sns.scatterplot(
#     data=df,
#     x="Compile Time (s)",
#     y="Solve Time (s)",
#     hue="Strategy",
#     style="Strategy",
# )
# plot.figure.savefig("output.pdf")
