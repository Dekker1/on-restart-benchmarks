include "rcpsp-wet.mzn";

annotation main_vars(array[int] of var int: vars);

solve
    ::int_search(s, first_fail, indomain_min, complete)
    minimize objective;
