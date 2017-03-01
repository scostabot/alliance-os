# Compile the I/O System Kernel
#
# HISTORY
# Date     Author    Rev    Notes
# 22/10/98 scosta    1      First draft
# 98/12/4  djarb     2      Minor kludges

CC=gcc
CPPFLAGS=-DSPARC -DSKTRACE
CFLAGS=-Wall -W -g
RM=rm -f

FLAGS= -Wall -o $@ -c
PPFLAGS= -I ../LM/include -I ../include -I include `orbit-config --cflags server`

# Part of LIBS will be set outside, so we guarantee omogeneous compile

LIBS= -L ../KL/String/binaries -L ../KL/Queue/binaries \
 -L ../LM/binaries -L ../KL/Threads/binaries \
 -llm -lstring -lqueue -lthread -lpthread \
 `orbit-config --libs server`

CLLIBS= `orbit-config --libs client`

OFLAGS= -o

# This is generic to all modules, so maybe defined externally, but in which way?
# CPPFLAGS will be set outside, so we guarantee omogeneous compile

objects/%.o: %.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= internal.c iosk-skels.c iosk-common.c iosk-impl.c

OBJS= objects/internal.o objects/iosk-skels.o \
 objects/iosk-common.o objects/iosk-impl.o

CLOBJ= objects/iosk-stubs.o objects/ioskclient.o objects/iosk-common.o

FINAL= ./binaries/iosk
TEST= ./binaries/ioskclient

all: $(FINAL) $(TEST)

$(FINAL): $(OBJS)
	$(CC) $(OBJS) $(LIBS) $(OFLAGS) $@

$(TEST) : $(FINAL) $(CLOBJ)
	$(CC) $(CLOBJ) $(CLLIBS) $(OFLAGS) $@

clean:
	$(RM) $(FINAL) $(TEST) $(OBJS) $(CLOBJ) *~
