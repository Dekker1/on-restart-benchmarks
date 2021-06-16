# Restart Based Meta-Search Experiments

This repository contains benchmarks of restart based meta-search presented in

> Dekker J.J., de la Banda M.G., Schutt A., Stuckey P.J., Tack G. (2018) Solver-Independent Large Neighbourhood Search. In: Hooker J. (eds) Principles and Practice of Constraint Programming. CP 2018. Lecture Notes in Computer Science, vol 11008. Springer, Cham. https://doi.org/10.1007/978-3-319-98334-9_6

and included in Jip J. Dekker's PhD Thesis.

These experiments concern the usage of propagation engine adjustments that capture some of the context of the current search space.
This context can later be accessed by special propagators to enforce properties that are interesting for meta-search.
Such as:

- The last value a variable has taken
- The last solution a variable has taken
- A random value, to be assigned by a variable

These benchmarks tests the effectiveness of this approach.
The experiments are conducted on the `gbac`, `rcpsp-wet` and `steelmillslab` problems from the MiniZinc challenge.
Each model is given two neighbourhoods for an round-robin LNS approach.

## Setup

The script `setup.sh` contains initial configuration of the experiments:

- The amount of time given for the test (120 sec)
- The amount of repeats for randomised search (10)
- It adds the correect version of MiniZinc to the `PATH`

The solver versions used are stored in the `software/` folder as git sub-modules.
The `install.sh` script can be used to compile the solvers and put them in the correct place.

## Run

The experiments can be run as different parts:

- Regular Gecode (base-line)
- Regular Chuffed (base-line)
- Restart Based Gecode (proposal)
- Restart Based Chuffed (proposal)
- Recording Gecode (data for replay)
- Replaying Gecode (ideal-case, no-computation)

For each part there is a `run_*.sh` script that will run the experiments and put its results into the `output` folder.
The `run_all.sh` script can be used to run them all in sequences.

## Analyse

The scripts `analyse_{gecode,chuffed}.py` can be used to process the information gathered during the experiments.
Each script is specific to that solver.
It takes the output folder of one of the runs as an argument (e.g., `output/gecode/gbac/original`).
The script produces a CSV format that can then be processed using software like Excel or Numbers.
