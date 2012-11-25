CC = gcc
DEBUGFLAGS = -ansi -Wall -pedantic -g
GOODFLAGS = -O3 -march=native

all: clean docs_serial docs_serialq docs_omp

docs_serial: docs-serial.c
	$(CC) $(GOODFLAGS) -fopenmp docs-serial.c -o docs-serial

docs_serialq: docs-serial-quick.c
	$(CC) $(GOODFLAGS) -fopenmp docs-serial-quick.c -o docs-serialq

docs_omp: docs-omp2.c
	$(CC) $(GOODFLAGS) -fopenmp docs-omp2.c -o docs-omp

backup: clean zip

clean:
	rm -f docs-serial docs-serialq docs-omp *.o

zip:
	@-tar -czf docs.tgz Makefile *.c report.pdf

.PHONY: clean zip
