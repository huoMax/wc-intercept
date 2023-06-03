CC = gcc
INCLUDE = -I./include/ -I./backend
SRC = ./src
OBJ = ./obj
BACKEND = ./backend
SAMPLE = ./sample

tracer : tracer.o wc_elf.o wc_backend.so
	$(CC)  $^ -o tracer -lelf -L.
	rm tracer.o wc_elf.o wc_ptrace.o wc_backend.o
	mv tracer $(OBJ)

breakpoint : $(SAMPLE)/breakpoint_tracer.c wc_elf.o
	$(CC) -c $(SAMPLE)/breakpoint_tracer.c $(INCLUDE) -o breakpoint.o
	$(CC) wc_elf.o breakpoint.o -o breakpoint -lelf
	rm wc_elf.o breakpoint.o


tracee : $(SAMPLE)/tracee.c
	$(CC) $^ -o tracee
	mv tracee $(OBJ)

wc_backend.so : $(BACKEND)/wc_backend.c wc_ptrace.o
	gcc -c -fPIC $(INCLUDE) $(BACKEND)/wc_backend.c -o wc_backend.o
	gcc -shared -fPIC wc_backend.o wc_ptrace.o -o wc_backend.so

wc_backend : $(BACKEND)/wc_backend.c wc_ptrace.o
	gcc -c -fPIC $(INCLUDE) $(BACKEND)/wc_backend.c -o wc_backend.o
	gcc -shared -fPIC wc_backend.o wc_ptrace.o -o wc_backend.so
	rm wc_ptrace.o wc_backend.o

wc_elf.o : $(SRC)/wc_elf.c
	$(CC) -c $^ $(INCLUDE) -o wc_elf.o

tracer.o : $(SRC)/tracer.c
	$(CC) -c $^ $(INCLUDE) -o tracer.o

wc_ptrace.o : $(BACKEND)/wc_ptrace.c
	$(CC) -c -fPIC $^ $(INCLUDE) -o wc_ptrace.o

clean:
	rm tracer.o wc_elf.o wc_ptrace.o tracer tracee

.PHONY: clean tracer tracee wc_backend breakpoint
