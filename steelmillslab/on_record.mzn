include "steelmillslab.mzn";
include "restart.mzn";

predicate int_eq_imp(var int: x, var int: y, var bool: b);
predicate bool_eq_imp(var bool: x, var bool: y, var bool: b);

predicate random_assignment(var bool: b) = forall(i in 1..nbSlabs) (
    int_eq_imp(assign[i], sol(assign[i]), b /\ (uniform_internal(1,100) < 80))
);

predicate random_bin(var bool: b) = let {
        var int: bin = uniform_internal(1, nbSlabs);
    } in forall(i in 1..nbSlabs) (
        int_eq_imp(assign[i], sol(assign[i]), b /\ (bin != sol(assign[i])))
    );

% Round Robin
array[1..2] of var bool: nbh;
constraint random_assignment(nbh[1]);
constraint random_bin(nbh[2]);
var 1..2: select;
constraint lastval(select) mod 2 + 1 = select; 
constraint bool_eq_imp(nbh[1], false, status() == START);
constraint bool_eq_imp(nbh[2], false, status() == START);
constraint bool_eq_imp(nbh[1], select == 1, status() != START);
constraint bool_eq_imp(nbh[2], select == 2, status() != START);

annotation main_vars(array[int] of var int: vars);

solve
    :: main_vars(assign)
    :: int_search(ordered, first_fail, indomain_min, complete)
    minimize objective;
