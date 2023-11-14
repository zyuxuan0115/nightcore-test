CC=gcc
CFLAGS=-I/usr/local/include -g -O0 -Wall
LINKER_FLAGS=-L/usr/local/lib
LIBUNWIND_FLAGS=-lunwind -lunwind-ptrace -lunwind-generic

all: tracer sort

tracer: tracer.c
	$(CC) $(CFLAGS) $^ -o $@ $(LINKER_FLAGS) $(LIBUNWIND_FLAGS)

sort: sort.c
	$(CC) -O1 $^ -o $@

clean:
	rm sort tracer
