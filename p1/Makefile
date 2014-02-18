all: gshell

gshell: gshell.o makeargv.o
	gcc -o gshell gshell.o makeargv.o

gshell.o: gshell.c
	gcc -c gshell.c


makeargv.o: makeargv.c
	gcc -c makeargv.c

clean:
	rm gshell *.o
