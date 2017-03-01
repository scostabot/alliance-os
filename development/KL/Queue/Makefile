# Compile the Alliance Kernel Library
#
# HISTORY
# Date     Author    Rev    Notes
# 12/11/98 S. Costa  1.0    First draft
# 12/12/00 scosta    1.1    More dependancies
# 21/12/03 scosta    1.2    Even more dependancies

CC=gcc
CFLAGS=-Wall -W -g
CPPFLAGS=-DSPARC -DSKTRACE
AR=ar
ARFLAGS=cruv
RM=rm -f
COMMONDEP=../../include/typewrappers.h ../../include/systemdefs.h ../../include/klibs.h

FLAGS= -o $@ -c
PPFLAGS= -Iinclude -I ../../include

objects/%.o: %.c $(COMMONDEP) 
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= QueueInit.c QueueAlloc.c QueueInsert.c QueueRemove.c QueueFree.c

OBJS= ./objects/QueueInit.o ./objects/QueueAlloc.o ./objects/QueueInsert.o\
      ./objects/QueueRemove.o ./objects/QueueFree.o

FINAL= ./binaries/libqueue.a

all: $(FINAL)

$(FINAL): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(FINAL) *~ ./include/*~

