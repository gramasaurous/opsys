all: shell

shell: shell.o makeargv.o
	gcc -o shell shell.o makeargv.o

shell.o: shell.c
	gcc -c shell.c

makeargv.o: makeargv.c
	gcc -c makeargv.c

clean:
	rm shell *.o

