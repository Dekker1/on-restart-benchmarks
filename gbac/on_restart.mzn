include "gbac.mzn";
include "restart.mzn";

predicate int_eq_imp(var int: x, var int: y, var bool: b);
predicate bool_eq_imp(var bool: x, var bool: y, var bool: b);

predicate random_allocation(var bool: b) = forall(i in courses) (
    int_eq_imp(period_of[i], sol(period_of[i]), b /\ (uniform_internal(1,100) < 80))
);

predicate free_period(var bool: b) = let {
        var int: period = uniform_internal(periods);
    } in forall(i in courses) (
        int_eq_imp(period_of[i], sol(period_of[i]), b /\ (sol(period_of[i]) != period))
    );

% Round Robin
array[1..2] of var bool: nbh;
constraint random_allocation(nbh[1]);
constraint free_period(nbh[2]);
var 1..2: select;
constraint lastval(select) mod 2 + 1 = select; 
constraint bool_eq_imp(nbh[1], false, status() == START);
constraint bool_eq_imp(nbh[2], false, status() == START);
constraint bool_eq_imp(nbh[1], select == 1, status() != START);
constraint bool_eq_imp(nbh[2], select == 2, status() != START);

annotation main_vars(array[int] of var int: vars);

solve
    :: main_vars(period_of)
    :: int_search(period_of,first_fail,indomain_min,complete) minimize objective;
