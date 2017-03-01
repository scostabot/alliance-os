# Compile the Alliance Loadable Module Library
#
# HISTORY
# Date     Author    Rev    Notes
# 12/11/98 S. Costa  1.0    First draft

CC=gcc
CPPFLAGS=-DSPARC -DSKTRACE
CFLAGS=-Wall -W -g
AR=ar
ARFLAGS=cruv
RM=rm -f

FLAGS=  -o $@ -c
PPFLAGS= -I include -I ../include

objects/%.o: %.c ../include/typewrappers.h ../KL/Exec/include/exec.h ../include/lm.h
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= LMlibInit.c LMload.c LMunload.c LMlibUninit.c

OBJS= ./objects/LMlibInit.o ./objects/LMload.o ./objects/LMunload.o ./objects/LMlibUninit.o

FINAL= ./binaries/liblm.a

all: $(FINAL)

$(FINAL): $(OBJS) 
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(FINAL) *~ core

