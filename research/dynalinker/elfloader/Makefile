# Makefile for dynamic ELF linkage
#
# HISTORY
# Date     Author    Rev    Notes
# 05/03/00 scosta    1.0    First draft

CC=/home/stefano/gcc3.3/bin/gcc
CFLAGS=-Wall -g
CPPFLAGS=-DI80X86 
AR=ar
ARFLAGS=cruv
RM=rm -f

PPFLAGS= -I../../../development/include -I ../../../development/KL/Exec/include -I include -DSKTRACE 
FLAGS= -o $@ -c

objects/%.o: %.c include/elfloader.h
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

SRCS=elfloader.c elflinker.c entrypoint.c \
     cpu.c statistics.c dumpfunc.c endian.c 

FINAL=./binaries/libdynlink.a

OBJS= ./objects/elfloader.o ./objects/elflinker.o \
      ./objects/entrypoint.o ./objects/cpu.o \
      ./objects/statistics.o ./objects/dumpfunc.o \
      ./objects/endian.o

LIBS=-L../../../development/KL/Monitor/binaries\
     -L../../../development/KL/String/binaries -lmonitor -lstring

OFLAGS=-o

$(FINAL): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) $(FINAL)  $(OBJS) *~ ./include/*~

