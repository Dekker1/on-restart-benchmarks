#!/usr/bin/env bash
set -e

module load Bison
module load CMake
module load flex

mkdir -p software/{build,install}

# Build 
for dir in "chuffed" "gecode" "gecode_base" "gecode_on_record" "gecode_on_replay" "minizinc"
do
	cmake -S software/${dir} -B software/build/${dir} -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=`pwd`/software/install/${dir}  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
	cmake --build software/build/${dir} --config Release --target install -- -j4
done

cmake -S software/mza -B software/build/mza -DGECODE_ROOT=`pwd`/software/install/gecode_base/ -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=`pwd`/software/install/mza -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build software/build/mza --config Release --target install -- -j4

ln -s `pwd`/software/install/gecode/share/minizinc/gecode/mznlib `pwd`/software/install/minizinc/share/minizinc/gecode
ln -s `pwd`/software/install/chuffed/share/minizinc/chuffed `pwd`/software/install/minizinc/share/minizinc/chuffed
