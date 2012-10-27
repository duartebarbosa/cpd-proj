CC = gcc
DEBUGFLAGS = -ansi -Wall -pedantic
GOODFLAGS = -O2 -march=native

docs_serial: docs-serial.c
	$(CC) $(GOODFLAGS) docs-serial.c -g -o docs-serial

all: clean docs_serial

backup: clean zip

clean:
	rm -f docs-serial *.o

zip:
	@-tar -czf docs.tgz Makefile *.c
	
.PHONY: clean zip
