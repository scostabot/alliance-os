# Compile the Alliance Kernel Library
#
# HISTORY
# Date     Author    Rev    Notes
# 12/11/98 S. Costa  1.0    First draft
# 21/12/03 scosta    1.1    Even more dependancies

CC=gcc
CPPFLAGS=-DSPARC -DSKTRACE
CFLAGS=-Wall -W -g
AR=ar
ARFLAGS=cruv
RM=rm -f
PLATFORM=SPARC
COMMONDEP=../../include/typewrappers.h ../../include/systemdefs.h ../../include/klibs.h

FLAGS= -o $@ -c
PPFLAGS= -I include -I ../../include

objects/%.o: %.c $(COMMONDEP) include/exec.h 
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

objects/%.o: ELF/%.c $(COMMONDEP) include/exec.h 
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

objects/%.o: ELF/architecture/$(PLATFORM)/%.c $(COMMONDEP) include/exec.h 
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= ExecClose.c ExecInit.c ExecLoad.c ExecOpen.c ExecProbe.c\
      ExecSetSymbols.c ELF/loader.c ELF/linker.c

OBJS= ./objects/ExecClose.o ./objects/ExecInit.o ./objects/ExecLoad.o\
      ./objects/ExecOpen.o ./objects/ExecProbe.o  ./objects/ExecSymbols.o\
      ./objects/ExecSetSymbols.o ./objects/cpu.o ./objects/loader.o \
      ./objects/linker.o

FINAL= ./binaries/libexec.a

all: $(FINAL)

$(FINAL): $(OBJS) 
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(FINAL) *~ ELF/*~ ELF/architecture/$(PLATFORM)/*~ core

