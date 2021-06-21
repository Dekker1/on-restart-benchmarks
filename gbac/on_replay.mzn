include "gbac.mzn";

annotation main_vars(array[int] of var int: vars);

solve 
    :: main_vars(period_of)
    :: int_search(period_of,first_fail,indomain_min,complete) minimize objective;
