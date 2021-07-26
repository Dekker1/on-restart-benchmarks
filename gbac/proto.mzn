include "gbac.mzn";
%-----------------------------------------------------------------------------%
% Objective
%-----------------------------------------------------------------------------%

function int: uniform_set(set of int: S) =
  if card(S) == max(S) - min(S) + 1 then
    uniform(min(S),max(S))
  else
    [ i | i in S ][uniform(1,card(S))]
  endif;


predicate random_allocation() = 
  forall(i in courses) (
    (uniform(1,100) < 80) -> (period_of[i] == sol(period_of[i]))
  );

predicate free_period() = 
  let {
    int: period = uniform_set(periods);
  } in forall(i in courses) (
    (sol(period_of[i]) != period) -> (period_of[i] == sol(period_of[i]))
  );

predicate LNS(int: choice) ::export =
  objective < sol(objective) /\
	if choice == 0 then
		free_period()
	else
		random_allocation()
	endif;

constraint output_this([objective] ++ period_of);
solve :: int_search(period_of,first_fail,indomain_min,complete)  satisfy;
