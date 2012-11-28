#!/bin/bash
# Bash script for running project tests

FILES=( ex5-1d ex10-2d ex1000-50d ex1M-100d )

make clean docs_omp
sync

for((i=0; i < 4; i++)) do
	echo "______"
	echo "input: "${FILES[i]}
	time ./docs-omp sampleDocInstances/in/${FILES[i]}.in
	cmp sampleDocInstances/in/${FILES[i]}.out sampleDocInstances/out/${FILES[i]}.out
done
exit
