include "gbac.mzn";

annotation main_vars(array[int] of var int: vars);

solve 
    :: int_search(period_of,first_fail,indomain_min,complete) minimize objective;
