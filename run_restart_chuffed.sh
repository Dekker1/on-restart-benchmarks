#!/bin/zsh
trap "exit" INT
set -e

folder=$1

source setup.sh

solver="./bin/fzn-chuffed"
model="./${folder}/on_restart"
output_folder="./output/chuffed/${folder}/restart"
mkdir -p ${output_folder}

for data in ./${folder}/*.dzn; do
	echo -n "Running ${model} with ${data}: "
	for i in {1..${runs}}; do
		echo -n "${i} "
		filename=$(basename -- "$data")
		filename="${filename%.*}"
		${minizinc}/mzn2fzn -Gchuffed ${model}.mzn ${data} &> ${output_folder}/${filename}.${i}.sol
		${solver} -a --time-out ${timeout_sec} --restart constant --restart-scale 250 -s --verbosity 2 --rnd-seed $i --restart-base 250 ${model}.fzn &>/dev/null | ${minizinc}/solns2out --output-time ${model}.ozn >> ${output_folder}/${filename}.${i}.sol
	done
	rm -f ${model}.fzn ${model}.ozn
	echo ""
done
