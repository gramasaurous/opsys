#Makefile
#Graham Greving

SERVERS = /usr/src/servers
FS 	= ${SERVERS}/fs
POSIX 	= /usr/src/lib/posix
TOOLS	= /usr/src/tools

useage: 
	@echo " " >&2
	@echo "Makefile to patch MINIX 3.1.2 filesystem encryption" >&2
	@echo " " >&2
	@echo "Useage:" >&2
	@echo "	make cplib 		# Copy setkey library files" >&2
	@echo "	make library 		# Make setkey library files" >&2
	@echo "	make cpsetkey 		# Copy setkey library files" >&2
	@echo "	make setkey		# Make setkey system call files " >&2
	@echo "	make boot		# Creates a new bootable image"
	@echo "	make test		# Compiles the setkey test code" >&2
	@echo " " >&2

all: cpsetkey setkey cplib library boot

boot:
	cd ${TOOLS} && make hdboot
	cd ${TOOLS} && make image
	ls -1 /boot/image
	
setkey:
	cd ${SERVERS} && make image
	cd ${SERVERS} && make install

cpsetkey:
	cp fs/* ${FS}

library:
	cd ${POSIX} && make Makefile
	cd /usr/src && make libraries

cplib:
	cp posix/setkey.c ${POSIX}/setkey.c
	cp posix/Makefile.in ${POSIX}/Makefile.in

test:
	cc -o test test.c

users:
	- adduser mike users /home/mike
	- adduser jane users /home/jane
	- adduser bob users /home/bob
	- adduser jill users /home/jill
	- adduser jeff users /home/jeff
	- adduser jackie users /home/jackie
	- adduser steve users /home/steve
	- adduser sarah users /home/sarah
