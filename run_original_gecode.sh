#!/bin/zsh
trap "exit" INT
set -e

folder=$1

source setup.sh

solver="./bin/fzn-gecode"
model="./${folder}/original"
output_folder="./output/gecode/${folder}/original"
mkdir -p ${output_folder}

for data in ./${folder}/*.dzn; do
	echo "Running ${model} with ${data}"
	filename=$(basename -- "$data")
	filename="${filename%.*}"
	${minizinc}/mzn2fzn -Ggecode ${model}.mzn ${data} &>${output_folder}/${filename}.sol
	${solver} --c-d 1 --a-d 2 -time ${timeout_sec}000 -a -s ${model}.fzn | ${minizinc}/solns2out --output-time ${model}.ozn >> ${output_folder}/${filename}.sol
	rm -f ${model}.fzn ${model}.ozn
done
