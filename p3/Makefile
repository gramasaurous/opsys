# SOURCES
CSOURCE		= buddy.c libmem.c libtest.c
CHEADER		= buddy.h libmem.h libtest.h
OBJECTS		= ${CSOURCE:.c=.o}
LIBOBJECTS 	= buddy.o libmem.o
EXECBIN		= libmem
TESTBIN		= libtest
LIBRARY		= libmem.a

# COMMANDS
GCC			= gcc
ARCHIVE		= ar rcs

test: ${TESTBIN}

${TESTBIN}: ${OBJECTS}
	${GCC} -o $@ ${OBJECTS}

%.o: %.c
	${GCC} -c $<

clean:
	- rm ${OBJECTS}

spotless: clean
	- rm ${TESTBIN}