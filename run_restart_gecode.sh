#!/bin/zsh
trap "exit" INT
set -e

folder=$1

source setup.sh

solver="./bin/fzn-gecode"
model="./${folder}/on_restart"
output_folder="./output/gecode/${folder}/restart"
mkdir -p ${output_folder}

for data in ./${folder}/*.dzn; do
	echo -n "Running ${model} with ${data}: "
	for i in {1..${runs}}; do
		echo -n "${i} "
		filename=$(basename -- "$data")
		filename="${filename%.*}"
		${minizinc}/mzn2fzn -Ggecode ${model}.mzn ${data} &> ${output_folder}/${filename}.${i}.sol
		${solver} --c-d 1 --a-d 2 -time ${timeout_sec}000 -r $i -a -restart constant -s ${model}.fzn | ${minizinc}/solns2out --output-time ${model}.ozn >> ${output_folder}/${filename}.${i}.sol
	done
	rm -f ${model}.fzn ${model}.ozn
	echo ""
done
