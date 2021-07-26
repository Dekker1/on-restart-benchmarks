#!/usr/bin/env python3
import os
import sys
import time
import csv

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
PROTO_MODEL = "gbac/proto.uzn"
RESTART_MODEL = "gbac/on_restart.mzn"
DATA = [
    "gbac/reduced_UD4-gbac.dzn",
    "gbac/UD2-gbac.dzn",
    "gbac/UD4-gbac.dzn",
    "gbac/UD5-gbac.dzn",
    "gbac/UD8-gbac.dzn",
]
FN_ID = "f_lex_obj_i"
N_NBH = 2
ROUNDS = 1000
RUNS = 1


def gbac_restart(data_file):
    os.environ["MZN_STDLIB_DIR"] = (
        os.getcwd() + "/software/install/minizinc/share/minizinc"
    )
    gecode = minizinc.Solver.lookup("gecode")
    inst = minizinc.Instance(gecode, minizinc.Model(RESTART_MODEL))
    inst.add_file(data_file)

    args = {
        "--restart": "constant",
        "--restart-scale": 500,
        "--restart-limit": ROUNDS,
    }
    res = inst.solve(**args)

    return (
        res.statistics["flatTime"].total_seconds(),
        (res.statistics["initTime"] + res.statistics["solveTime"]).total_seconds(),
    )


def gbac_incr(data_file):
    os.environ["MZN_STDLIB_DIR"] = os.getcwd() + "/software/mza/share/minizinc"
    compile_time = 0.0
    solve_time = 0.0

    incumbent = None
    start = time.perf_counter()
    mza.set_rnd_seed(0)
    inst = Instance(PROTO_MODEL, data_file, "gecode_presolver")
    compile_time += time.perf_counter() - start

    start = time.perf_counter()
    status, sol = inst.solve()
    solve_time += time.perf_counter() - start
    # print(f"{status}: {sol}")
    assert status in ["SAT", "OPT"]
    incumbent = sol
    for i in range(1, ROUNDS):
        mza.set_rnd_seed(i)
        inst.set_limit(500)
        with inst.branch() as child:
            start = time.perf_counter()
            child.add_call("f_LNS_i", i % 2)
            compile_time += time.perf_counter() - start

            start = time.perf_counter()
            status, sol = child.solve()
            solve_time += time.perf_counter() - start
            # print(f"{status}: {sol}")
            if status == "SAT" or status == "OPT":
                incumbent = sol
            assert status != "ERROR"
    return compile_time, solve_time


def gbac_redo(data_file):
    os.environ["MZN_STDLIB_DIR"] = os.getcwd() + "/software/mza/share/minizinc"
    compile_time = 0.0
    solve_time = 0.0

    incumbent = None
    for i in range(ROUNDS):
        start = time.perf_counter()
        mza.set_rnd_seed(i)
        inst = Instance(PROTO_MODEL, data_file, "gecode_presolver")
        inst.output_dict(True)
        limit = 0
        if i > 0:
            inst.set_limit(500)
            assert incumbent is not None
            limit = 500
            inst.set_incumbent(incumbent)
            inst.add_call("f_LNS_i", i % 2)
        compile_time += time.perf_counter() - start

        start = time.perf_counter()
        status, sol = inst.solve()
        solve_time += time.perf_counter() - start
        # print(
        #     f"{i} {status}: [{ ', '.join([str(v) for k, v in sol.items()]) if isinstance(sol, dict) else sol}]"
        # )
        if status == "SAT" or status == "OPT":
            incumbent = sol
        elif status == "ERROR":
            print("ERROR!!!!")
            exit(0)
    return compile_time, solve_time


if __name__ == "__main__":
    fieldnames = ["Configuration", "Data", "Compile Time (s)", "Solve Time (s)"]
    writer = csv.writer(sys.stdout)
    writer.writerow(fieldnames)

    # --- Run Restart based strategy
    for d in DATA:
        t1, t2 = 0, 0
        for i in range(RUNS):
            ct, st = gbac_restart(d)
            t1 += ct
            t2 += st
        writer.writerow(["RBS", d, t1 / RUNS, t2 / RUNS])
    # --- Run incremental rewriting
    for d in DATA:
        t1, t2 = 0, 0
        for i in range(RUNS):
            ct, st = gbac_incr(d)
            t1 += ct
            t2 += st
        writer.writerow(["Incr.", d, t1 / RUNS, t2 / RUNS])
    # --- Run baseline
    for d in DATA:
        t1, t2 = 0, 0
        for i in range(RUNS):
            ct, st = gbac_redo(d)
            t1 += ct * 10
            t2 += st * 10
        writer.writerow(["Base", d, t1 / RUNS, t2 / RUNS])
