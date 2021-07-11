# Incremental Meta-Search Experiments

This repository contains benchmarks of meta-search through incremental usage of MiniZinc, included in Jip J. Dekker's PhD Thesis and partially presented in the following paper.

> Dekker J.J., de la Banda M.G., Schutt A., Stuckey P.J., Tack G. (2018) Solver-Independent Large Neighbourhood Search. In: Hooker J. (eds) Principles and Practice of Constraint Programming. CP 2018. Lecture Notes in Computer Science, vol 11008. Springer, Cham. https://doi.org/10.1007/978-3-319-98334-9_6

These experiments concern two model for the incremental solving of MiniZinc models:
- The usage of propagation engine adjustments that capture some of the context of the current search space. This context can later be accessed by special propagators to enforce properties that are interesting for meta-search.  Such as:
	- The last value a variable has taken
	- The last solution a variable has taken
	- A random value, to be assigned by a variable
- An incremental rewriting engine for MiniZinc instances that minimizes overhead of incremental changes.

These benchmarks tests the effectiveness of these approaches.
The experiments are conducted on the `gbac`, `radiation`, `rcpsp-wet` and `steelmillslab` problems from the MiniZinc challenge.
The `radiation` model uses a lexicographic objective.
The other models are solved using two neighbourhoods for an round robin LNS approach.

## Setup

The script `setup.sh` contains initial configuration of the experiments:

- The amount of time given for the test (120 sec)
- The amount of repeats for randomised search (10)
- It adds the correct version of MiniZinc to the `PATH`

The solver versions used are stored in the `software/` folder as git sub-modules.
The `install.sh` script can be used to compile the solvers and put them in the correct place.

## Run

### Large Neighbourhood Search

The experiments can be run as different parts:

- Regular Gecode (baseline)
- Regular Chuffed (baseline)
- Restart Based Gecode (proposal)
- Restart Based Chuffed (proposal)
- Recording Gecode (data for replay)
- Replaying Gecode (ideal case, no computation)

For each part there is a `run_*.sh` script that will run the experiments and put its results into the `output` folder.
The `run_all.sh` script can be used to run them all in sequences.

### Effort Comparison

## Analyse

### Large Neighbourhood Search

The scripts `analyse_{gecode,chuffed}.py` can be used to process the information gathered during the experiments.
Each script is specific to that solver.
It takes the output folder of one of the models as an argument (e.g., `output/gecode/gbac`).
The script produces a CSV format that can then be processed using software like Excel or Numbers.

The script `cumulative_analysis.py` can generate a PDF plot `output.pdf` for of the cumulative objective value over time.
It takes the output folder of one of the models and the name of the solver as an argument.

### Effort Comparison
