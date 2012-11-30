#!/bin/bash
# Bash script for running project tests

rm -rf log
clear
mkdir log

bash test-serial.sh > log/serial.log
bash test-omp.sh > log/omp.log
bash test-mpi.sh > log/mpi.log
bash test-mpi-omp.sh > log/mpi-omp.log

make clean
exit
