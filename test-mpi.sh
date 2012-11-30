#!/bin/bash
# Bash script for running project tests

FILES=( ex5-1d ex10-2d ex1000-50d ex1M-100d )

make clean docs_mpi
sync

for((i=0; i < 4; i++)) do
	echo "_________________________________________"
	echo "input: "${FILES[i]}
	time /usr/lib64/openmpi/bin/mpirun -np 4 ./docs-mpi sampleDocInstances/in/${FILES[i]}.in
	cmp sampleDocInstances/in/${FILES[i]}.out sampleDocInstances/out/${FILES[i]}.out
done
exit
