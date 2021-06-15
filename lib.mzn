/***
	@groupdef internal Internal Builtins

	These annotations and functions provide the basic building blocks to implement
	meta-heuristics and governed search with the use of solver internal values and
	functionality.
*/

/*
	A search annotation that supplies a function to be executed on restart
*/
ann: on_restart(string: pred);

/*
	The 'status' function reports the status of the solver (before restarting).
*/
enum STATUS = {UNKNOWN, UNSAT, SAT, OPT};
function var STATUS: status();

function var int: restart_number();

/*
	The 'rnd' function provides random values chosen by the solver. The arguments
	to the function limit the values to a certain domain.
*/
function var int: uniform_internal(int: i, int: j) ::impure = int_uniform(i, j);

function var int: uniform_internal(set of int: S) ::impure =
	if card(S) == max(S) - min(S) + 1 then
		int_uniform(min(S),max(S))
	else
		[ i | i in S ][int_uniform(1,card(S))]
	endif;

/*
	'int_rnd' is the random intrinsic that needs to be implemented by the solver.
*/
function var int: int_uniform(int: a, int: b) ::impure;

/*
	The 'sol' functions provides access to solution values of model variables. The
	sol functions are only safe to use when the solver status is not UNKNOWN.
*/
function var bool: sol(var bool: x) =
	if is_fixed(x) then
		fix(x)
	else
		bool_sol(x)
	endif;

function var float: sol(var float: x) =
	if is_fixed(x) then
		fix(x)
	else
		float_sol(x)
	endif;

function var int: sol(var int: x) =
	if is_fixed(x) then
		fix(x)
	else
		int_sol(x)
	endif;

function array[int] of var bool: sol(array[int] of var bool: x) =
	[if is_fixed(x[i]) then fix(x[i]) else bool_sol(x[i]) endif | i in index_set(x)];

function array[int] of var float: sol(array[int] of var float: x) =
	[if is_fixed(x[i]) then fix(x[i]) else float_sol(x[i]) endif | i in index_set(x)];

function array[int] of var int: sol(array[int] of var int: x) =
	[if is_fixed(x[i]) then fix(x[i]) else int_sol(x[i]) endif | i in index_set(x)];

/*
	'bool_rnd', 'float_sol', and 'int_sol' are the solution intrinsics that needs
	to be implemented by the solver.
*/
function var bool: bool_sol(var bool: a);
function var float: float_sol(var float: a);
function var int: int_sol(var int: a);

/*
	'round_robin' provides a metaheuristic for LNS where each neighbourhood is
	chosen sequentially.
*/
predicate round_robin(array[int] of var bool: nbhs) =
	let {
		set of int: N = index_set(nbhs);
		% Neighbourhood control variables
		array[N] of var bool: control;
		% Neighbourhood selection
		var N: select;
	} in forall(i in N) (control[i] -> nbhs[i]) /\
		if status() == UNKNOWN then
			control = [false | i in N]
			/\ select = max(N)
		else
			select = (sol(select) + 1) mod card(N)
			/\ control = [i == select | i in N]
		endif;
