# Compile the Alliance Kernel Library
#
# HISTORY
# Date     Author    Rev    Notes
# 12/11/98 S. Costa  1.0    First draft
# 12/12/00 scosta    1.1    More dependancies
# 21/12/03 scosta    1.2    Even more dependancies

CC=gcc
CPPFLAGS=-DSPARC -DSKTRACE
CFLAGS=-Wall -W -g
AR=ar
ARFLAGS=cruv
RM=rm -f
COMMONDEP=../../include/typewrappers.h ../../include/systemdefs.h ../../include/klibs.h

FLAGS= -o $@ -c
PPFLAGS= -I ../../include

# This is generic to all modules, so maybe defined externally, but in which way?
# CPPFLAGS will be set outside, so we guarantee omogeneous compile

objects/%.o: %.c $(COMMONDEP) 
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= MemoryCompare.c MemoryCopy.c MemorySet.c

OBJS= ./objects/MemoryCompare.o ./objects/MemoryCopy.o ./objects/MemorySet.o

FINAL= ./binaries/libmemory.a

all: $(FINAL)

$(FINAL): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(FINAL) *~

