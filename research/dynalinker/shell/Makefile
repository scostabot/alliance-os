# Makefile for research shell for dynalinking
#
# HISTORY
# Date     Author    Rev    Notes
# 05/03/00 scosta    1.0    First draft
# 14/10/00 scosta    1.1    Added support for memory align
# 23/10/00 scosta    1.2    Corrected dependancies and test
# 08/11/03 scosta    1.3    Better shell code handling

CC=/home/stefano/gcc3.3/bin/gcc
CFLAGS=-Wall -g
CPPFLAGS=-DI80X86 
RM=rm -f

PPFLAGS= -I ../../../development/include -I ../elfloader/include \
         -I include

FLAGS= -o $@ -c

objects/%.o: %.c include/shell.h ../../../development/include/klibs.h
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

SRCS=shell.c cmddynalinker.c

FINAL=./binaries/dynsh

OBJS= ./objects/shell.o ./objects/cmddynalinker.o

LIBS=-L../../../development/KL/Monitor/binaries\
     -L../../../development/KL/Memory/binaries \
     -L../../../development/KL/String/binaries \
     -L../elfloader/binaries \
     -ldynlink -lmonitor -lstring -lmemory

OFLAGS=-o

$(FINAL): $(OBJS) ../elfloader/binaries/libdynlink.a
	$(CC) $(OBJS) $(LIBS) $(OFLAGS) $@

TESTDIR=../TestSuite/objects/architecture/$(PLATFORM)

info:
	$(FINAL) $(TESTDIR)/

clean:
	$(RM) $(FINAL) $(OBJS) core *~

