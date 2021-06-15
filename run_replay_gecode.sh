#!/usr/bin/env zsh
trap "exit" INT
set -e

folder=$1

source setup.sh

solver="./bin/fzn-gecode_replay"
model="./${folder}/original"
output_folder="./output/gecode/${folder}/replay"
records_folder="./output/gecode/${folder}/record"
mkdir -p ${output_folder}

for data in ./${folder}/*.dzn; do
	echo -n "Replaying ${model} with ${data}: "
	for i in {1..${runs}}; do
		echo -n "${i} "
		filename=$(basename -- "$data")
		filename="${filename%.*}"
		cp ${records_folder}/${filename}.${i}.rec record.txt 
		${minizinc}/mzn2fzn -Ggecode ${model}.mzn ${data} &> ${output_folder}/${filename}.${i}.sol
		${solver} --c-d 1 --a-d 2 -time ${timeout_sec}000 -r $i -a -restart constant -s ${model}.fzn | ${minizinc}/solns2out --output-time ${model}.ozn >> ${output_folder}/${filename}.${i}.sol
	done
	rm -f ${model}.fzn ${model}.ozn
	echo ""
done
