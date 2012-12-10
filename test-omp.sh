#!/bin/bash
# Bash script for running project tests

THREADS=( 1 2 4 8 )
FILES=( ex5-1d ex10-2d ex1000-50d ex1M-100d ex100k-200-3 ex100k-200-4-mod )

make clean docs_omp
sync

for((j=0; j < 3; j++)) do
	export OMP_NUM_THREADS=${THREADS[j]}
	echo "threads: "${THREADS[j]}
	for((i=0; i < 5; i++)) do
		echo "_________________________________________"
		echo "input: "${FILES[i]}
		time ./docs-omp sampleDocInstances/in/${FILES[i]}.in
		cmp sampleDocInstances/in/${FILES[i]}.out sampleDocInstances/out/${FILES[i]}.out
	done
done
exit
