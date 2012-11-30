CC = gcc
DEBUGFLAGS = -ansi -Wall -pedantic -g
GOODFLAGS = -O3 -march=native
TIMER = -DGETTIME=1

all: clean docs_serial docs_omp docs_mpi docs_mpi_omp

docs_serial: docs-serial.c
	$(CC) $(GOODFLAGS) $(TIMER) -fopenmp docs-serial.c -o docs-serial

docs_omp: docs-omp.c
	$(CC) $(GOODFLAGS) $(TIMER) -fopenmp docs-omp.c -o docs-omp

docs_mpi: docs-mpi.c
	/usr/lib64/openmpi/bin/mpicc $(GOODFLAGS) $(TIMER) -g docs-mpi.c -o docs-mpi

docs_mpi_omp: docs-mpi-omp.c
	/usr/lib64/openmpi/bin/mpicc $(GOODFLAGS) $(TIMER) -fopenmp -g docs-mpi-omp.c -o docs-mpi-omp

backup: clean zip

clean:
	rm -f docs-serial docs-omp docs-mpi docs-mpi-omp *.o

zip:
	@-tar -czf docs.tgz Makefile *.c report.pdf

.PHONY: clean zip
