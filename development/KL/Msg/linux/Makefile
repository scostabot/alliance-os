# Compile the Alliance EK Alloc Library
#
# HISTORY
# Date     Author    Rev    Notes
# 14/03/99 Jens      1.0    First Release

CFLAGS= -Wall -DI80X86 -DREAL_ALLIANCE
FLAGS= -O3 -o $@ -c -g
PPFLAGS= -I../../../include -I./
LDFLAGS= -L../../Alloc/binaries/ -lalloc

# This is generic to all modules, so maybe defined externally, but in which way?
# CPPFLAGS will be set outside, so we guarantee omogeneous compile

objects/%.o: %.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

LIBO= ./objects/msg.o ./objects/signal.o
BINO= ./objects/demo.o
OBJS= $(BINO) $(LIBO)

FINAL= ./binaries/libmsg.a
BINSO= ./binaries/demo

all: $(FINAL) $(BINSO)

$(BINSO): $(FINAL)
	$(CC) -o $(BINSO) $(BINO) $(FINAL) $(LDFLAGS)

$(FINAL): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBO)

clean:
	$(RM) $(OBJS) $(FINAL) $(BINSO) *~
