CC = gcc
DEBUGFLAGS = -ansi -Wall -pedantic -g
GOODFLAGS = -O3 -march=native

all: clean docs_serial docs_omp docs_mpi

docs_serial: docs-serial.c
	$(CC) $(GOODFLAGS) -fopenmp docs-serial.c -o docs-serial

docs_omp: docs-omp.c
	$(CC) $(GOODFLAGS) -fopenmp -g docs-omp.c -o docs-omp

docs_mpi: docs-mpi.c
	$(CC) $(GOODFLAGS) -fopenmp -g docs-mpi.c -o docs-mpi

backup: clean zip

clean:
	rm -f docs-serial docs-omp docs-mpi *.o

zip:
	@-tar -czf docs.tgz Makefile *.c report.pdf

.PHONY: clean zip
