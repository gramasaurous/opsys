#
# Makefile for Rijndael (AES) code.
#
# $Id: Makefile,v 1.2 2003/04/15 00:26:33 elm Exp elm $
# NOTE: has GNU make specific stuff

SRCS = encrypt.c rijndael.c
INCS = rijndael.h
PROGS = encrypt
OTHERS = testfile Makefile
DIR = $(notdir $(PWD))
TAR = tar
TARFILE = aes.tgz

COBJS = rijndael.o
CFLAGS = -O3

all: $(PROGS)

encrypt: encrypt.o $(COBJS) $(INCS)
	$(CC) $(CFLAGS) -o $@ encrypt.o $(COBJS)

rijndael.o: rijndael.h

tarball:
	cd .. ; $(TAR) cvf - $(addprefix $(DIR)/, $(SRCS) $(INCS) $(OTHERS)) | gzip - > $(TARFILE)

test: all
	cp testfile testfile.orig~
	./encrypt 0x1234 0x5678 testfile
	./encrypt 0x1234 0x5678 testfile
	diff testfile testfile.orig~

clean:
	$(RM) *~ *.o

spotless: clean
	$(RM) $(PROGS)