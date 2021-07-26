include "radiation.mzn";
include "restart.mzn";

%-----------------------------------------------------------------------------%
% Objective
%-----------------------------------------------------------------------------%

predicate complete();% = abort("'complete' should be used in reified context");
predicate complete_reif(var bool: marker);

predicate lex_minimize(array[int] of var int: o) =
    let {
        var min(index_set(o))..max(index_set(o))+1: stage;
    } in if status() = START then
        stage = min(index_set(o))
    elseif status() = UNSAT then
        stage = lastval(stage) + 1
    else /* status() = SAT */
        stage = lastval(stage)
        /\ o[stage] < sol(o[stage])
    endif
    /\ forall(i in min(index_set(o))..stage-1) (
        o[i] = sol(o[i])
    )
    /\ if stage > max(index_set(o)) then
        complete()
    endif;

% var STATUS: status ::add_to_output = status();
constraint lex_minimize([Beamtime, K]);

solve :: int_search([Beamtime] ++ N ++ [Q[i,j,b] | i in Rows, j in Columns, b in BTimes ], input_order, indomain_split, complete) satisfy;
