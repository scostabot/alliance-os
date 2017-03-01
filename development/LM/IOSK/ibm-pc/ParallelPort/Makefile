# Compile the Parallel Port LM
#
# HISTORY
# Date     Author    Rev    Notes
# 25/03/99 S. Costa  1.0    First draft

CC=gcc
CPPFLAGS=-DSPARC -DSKTRACE
CFLAGS=-Wall -W -g
RM=rm -f

FLAGS= -O2 -o $@ -c
PPFLAGS= -D SKTRACE -I ../../../include -I ../../../../include\
         -I ../../../../CK/include/sys/

# This is generic to all modules, so maybe defined externally, but in which way?
# CPPFLAGS will be set outside, so we guarantee omogeneous compile

objects/%.o: %.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= LMparallel.c UNIXstub.c

OBJS= objects/LMparallel.o objects/UNIXstub.o

LIBS = -L../../../../KL/String/binaries\
 -L../../../../KL/Monitor/binaries -lstring -lmonitor

FINAL= ./binaries/LMparallel
OFLAGS= -o

all: $(FINAL)

$(FINAL): $(OBJS)
	$(CC) $(OBJS) $(LIBS) $(OFLAGS) $@

clean:
	$(RM) $(OBJS) $(FINAL) *~

