#!/bin/bash
# Bash script for running project tests

PROCESSES=( 1 2 4 8 )
FILES=( ex5-1d ex10-2d ex1000-50d ex1M-100d ex100k-200-3 ex100k-200-4-mod )

make clean docs_mpi
sync

for((j=0; j < 3; j++)) do
	echo "processes: "${PROCESSES[j]}
	for((i=0; i < 5; i++)) do
		echo "_________________________________________"
		echo "input: "${FILES[i]}
		time /usr/lib64/openmpi/bin/mpirun -np ${PROCESSES[j]} ./docs-mpi sampleDocInstances/in/${FILES[i]}.in
		cmp sampleDocInstances/in/${FILES[i]}.out sampleDocInstances/out/${FILES[i]}.out
	done
done
exit
