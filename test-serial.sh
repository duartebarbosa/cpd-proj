#!/bin/bash
# Bash script for running project tests

FILES=( ex5-1d ex10-2d ex1000-50d ex1M-100d ex100k-200-3 ex100k-200-4-mod )

make clean docs_serial
sync

for((i=0; i < 6; i++)) do
	echo "_________________________________________"
	echo "input: "${FILES[i]}
	time ./docs-serial sampleDocInstances/in/${FILES[i]}.in
	cmp sampleDocInstances/in/${FILES[i]}.out sampleDocInstances/out/${FILES[i]}.out
done
exit
