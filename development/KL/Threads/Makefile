# Compile the Alliance Kernel Library
#
# HISTORY
# Date     Author    Rev    Notes
# 12/11/98 S. Costa  1.0    First draft

CC=gcc
CFLAGS=-Wall -W -g
CPPFLAGS=-DSPARC -DSKTRACE
AR=ar
ARFLAGS=cruv
RM=rm -f

# IMHO the -DPENTIUM is utter garbage....for now I'll leave it here.
# Candidate for deletion after ./configure

FLAGS= -o $@ -c
PPFLAGS= -I ../../include -DREAL_ALLIANCE

# This is generic to all modules, so maybe defined externally, but in which way?
# CPPFLAGS will be set outside, so we guarantee omogeneous compile

objects/%.o: %.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= Mutex.c Thread.c

OBJS= ./objects/Mutex.o ./objects/Thread.o

FINAL= ./binaries/libthread.a

all: $(FINAL)

$(FINAL): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(FINAL) *~

