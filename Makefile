CC = gcc
DEBUGFLAGS = -ansi -Wall -pedantic -g
GOODFLAGS = -O3 -march=native

all: clean docs_serial docs_omp

docs_serial: docs-serial.c
	$(CC) $(GOODFLAGS) -fopenmp docs-serial.c -o docs-serial

docs_omp: docs-omp.c
	$(CC) $(GOODFLAGS) -fopenmp docs-omp.c -o docs-omp

backup: clean zip

clean:
	rm -f docs-serial docs-omp *.o

zip:
	@-tar -czf docs.tgz Makefile *.c report.pdf

.PHONY: clean zip
