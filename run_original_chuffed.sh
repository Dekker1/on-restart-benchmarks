#!/bin/zsh
trap "exit" INT
set -e

folder=$1

source setup.sh

solver="./software/install/chuffed/bin/fzn-chuffed"
model="./${folder}/original"
output_folder="./output/chuffed/${folder}/original"
mkdir -p ${output_folder}

for data in ./${folder}/*.dzn; do
	echo "Running ${model} with ${data}"
	filename=$(basename -- "$data")
	filename="${filename%.*}"
	minizinc --solver mzn-fzn -c -Gchuffed ${model}.mzn ${data} &> ${output_folder}/${filename}.sol
	${solver} -a --time-out ${timeout_sec} -s --verbosity 2 ${model}.fzn &>/dev/null | minizinc --output-time --ozn-file ${model}.ozn >> ${output_folder}/${filename}.sol
	rm -f ${model}.fzn ${model}.ozn
done
