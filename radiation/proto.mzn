include "radiation.mzn";
include "lex_less.mzn";

%-----------------------------------------------------------------------------%
% Objective
%-----------------------------------------------------------------------------%

predicate lex_minimize(array[int] of var int: o, int: stage) =
	forall(i in min(index_set(o))..stage-1) (
		o[i] = sol(o[i])
	)
	/\ o[stage] < sol(o[stage]);

constraint output_this([Beamtime, K]);
predicate lex_obj(int: stage) ::export = lex_minimize([Beamtime, K], stage);

solve :: int_search([Beamtime] ++ N ++ [Q[i,j,b] | i in Rows, j in Columns, b in BTimes ], input_order, indomain_min, complete) satisfy;
