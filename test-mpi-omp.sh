#!/bin/bash
# Bash script for running project tests

PROCESSES=( 1 2 4 8 )
THREADS=( 1 2 4 8 )
FILES=( ex5-1d ex10-2d ex1000-50d ex1M-100d ex100k-200-3 ex100k-200-4-mod )

make clean docs_mpi_omp
sync

for((k=0; k < 4; k++)) do
	echo "processes: "${PROCESSES[k]}
	for((j=0; j < 4; j++)) do
		export OMP_NUM_THREADS=${THREADS[j]}
		echo "threads: "${THREADS[j]}
		for((i=0; i < 6; i++)) do
			echo "_________________________________________"
			echo "input: "${FILES[i]}
			time /usr/lib64/openmpi/bin/mpirun -np ${PROCESSES[k]} ./docs-mpi sampleDocInstances/in/${FILES[i]}.in
			cmp sampleDocInstances/in/${FILES[i]}.out sampleDocInstances/out/${FILES[i]}.out
		done
	done
done
exit
