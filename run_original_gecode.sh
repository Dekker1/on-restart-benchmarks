#!/usr/bin/env bash
trap "exit" INT
set -e

folder=$1

source setup.sh

solver="./software/install/gecode/bin/fzn-gecode"
model="./${folder}/original"
output_folder="./output/gecode/${folder}/original"
mkdir -p ${output_folder}

for data in ./${folder}/*.dzn; do
	echo "Running ${model} with ${data}"
	filename=$(basename -- "$data")
	filename="${filename%.*}"
	minizinc --solver mzn-fzn -c -Ggecode ${model}.mzn ${data} &>${output_folder}/${filename}.sol
	${solver} --c-d 1 --a-d 2 -time ${timeout_sec}000 -a -s ${model}.fzn | minizinc --output-time --ozn-file ${model}.ozn >> ${output_folder}/${filename}.sol
	rm -f ${model}.fzn ${model}.ozn
done
