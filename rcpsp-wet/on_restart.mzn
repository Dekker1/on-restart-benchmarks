include "rcpsp-wet.mzn";
include "restart.mzn";

predicate int_eq_imp(var int: x, var int: y, var bool: b);
predicate bool_eq_imp(var bool: x, var bool: y, var bool: b);

predicate randomize(var bool: b) =
    forall(t in Tasks) (
        int_eq_imp(s[t], sol(s[t]), b /\ uniform_internal(1,100) < 80)
    );

predicate free_timeslot(var bool: b) =
    let {
        int: slot = max(Times) div 10;
        var int: time = uniform_internal(min(Times), max(Times) - slot);
    } in forall(t in Tasks) (
        int_eq_imp(s[t], sol(s[t]), b /\ (sol(s[t]) < time \/ time+slot > sol(s[t])))
    );

% Round Robin
array[1..2] of var bool: nbh;
constraint randomize(nbh[1]);
constraint free_timeslot(nbh[2]);
var 1..2: select;
constraint lastval(select) mod 2 + 1 = select; 
constraint bool_eq_imp(nbh[1], false, status() == START);
constraint bool_eq_imp(nbh[2], false, status() == START);
constraint bool_eq_imp(nbh[1], select == 1, status() != START);
constraint bool_eq_imp(nbh[2], select == 2, status() != START);

annotation main_vars(array[int] of var int: vars);

solve
    ::int_search(s, first_fail, indomain_min, complete)
    minimize objective;
