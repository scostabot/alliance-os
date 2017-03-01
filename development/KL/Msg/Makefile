# Compile the Alliance KL Alloc Library
#
# HISTORY
# Date     Author    Rev    Notes
# 14/03/99 Jens      1.0    First Release

CC=gcc
CFLAGS=-Wall -W -g
CPPFLAGS=-DSPARC -DSKTRACE
AR=ar
ARFLAGS=cruv
RM=rm -f

FLAGS= -O3 -o $@ -c
PPFLAGS= -I ../../include

# This is generic to all modules, so maybe defined externally, but in which way?
# CPPFLAGS will be set outside, so we guarantee omogeneous compile

objects/%.o: %.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= FreelistAlloc.c BitmapAlloc.c

OBJS= ./objects/FreelistAlloc.o ./objects/BitmapAlloc.o

FINAL= ./binaries/liballoc.a

all: # $(FINAL)

$(FINAL): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(FINAL) *~
