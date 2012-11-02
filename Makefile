CC = gcc
DEBUGFLAGS = -ansi -Wall -pedantic -g
GOODFLAGS = -O2 -march=native

docs_serial: docs-serial.c
	$(CC) $(GOODFLAGS) docs-serial.c -o docs-serial

docs_omp: docs-omp.c
	$(CC) $(GOODFLAGS) -fopenmp docs-omp.c -o docs-omp

all: clean docs_serial docs_omp

backup: clean zip

clean:
	rm -f docs-serial docs-omp *.o

zip:
	@-tar -czf docs.tgz Makefile *.c report.tex
	
.PHONY: clean zip
