include "steelmillslab.mzn";

annotation main_vars(array[int] of var int: vars);

solve
    :: int_search(ordered, first_fail, indomain_min, complete)
    minimize objective;
