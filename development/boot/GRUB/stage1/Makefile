############################################################################
#  GRUB stage1 makefile
#

CC=gcc
CFLAGS= $(SHARED_FLAGS)
OBJCOPY=objcopy
LD=/usr/ccs/bin/ld
RM=rm -f

all: ../binaries/stage1

../objects/stage1.o:	stage1.S Makefile ../Makefile
	$(CC) $(CFLAGS) -c -o ../objects/stage1.o stage1.S

../objects/stage1.elf:	../objects/stage1.o Makefile ../Makefile
	$(LD) -N -Ttext 7C00 -o ../objects/stage1.elf ../objects/stage1.o

../binaries/stage1:	../objects/stage1.elf Makefile ../Makefile
	$(OBJCOPY) -O binary ../objects/stage1.elf ../binaries/stage1

clean:
	$(RM) ../objects/stage1.o ../objects/stage1.elf
