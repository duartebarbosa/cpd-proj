#!/bin/bash
# Bash script for running project tests

FILES=( ex1000-50d ex10-2d ex1M-100d ex5-1d )

echo "Compiling"
make all;
sync;

echo "Executing"
for((i=0; i < 4; i++)) do
	echo "	input: "${FILES[i]}
	time ./docs-serial sampleDocInstances2/${FILES[i].in}
done

echo "Testing"
for((i=0; i < 4; i++)) do
	diff sampleDocInstances/${FILES[i]}.out sampleDocInstances2/${FILES[i]}.out
done

echo "Cleaning"
make clean;

echo "Done!"
exit
