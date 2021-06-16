#!/usr/bin/env bash
set -e

module load Bison
module load CMake
module load flex

mkdir -p software/{build,install}

# Build Chuffed
for dir in "chuffed" "gecode" "gecode_on_record" "gecode_on_replay" "minizinc"
do
	cmake -S software/${dir} -B software/build/${dir} -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=`pwd`/software/install/${dir}
	cmake --build software/build/${dir} --config Release --target install -- -j4
done

