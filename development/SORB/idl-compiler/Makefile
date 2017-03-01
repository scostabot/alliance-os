# Compile the Alliance IDL compiler
#
# HISTORY
# Date     Author Rev Notes
# 24/12/98 Luc    1   Based on IOSK makefile. Happy Christmans!

#defined outside, should be removed
CC=egcs
CFLAGS=-Wall -g
PLATFORM=I80X86
CPPFLAGS=-DI80X86
# done

RM=rm -f
BUILDROOT=../..
FLAGS= -Wall -o $@ -c
PPFLAGS= -I ./include -I ../include -I ../libIDL -I $(BUILDROOT)/include \
            `glib-config --cflags`

SRCS=main.c hFile.c utils.c toString.c
OBJS=objects/main.o objects/hFile.o objects/utils.o objects/toString.o
FINAL=binaries/idlc

# Dynamically linking libIDL... can cause problems if an old version is
# found earlier on the dynamic linker search path.
#LIBS=-L../libIDL/.libs -lIDL `glib-config --libs` \
#     -L $(BUILDROOT)/KL/String/binaries -lstring

# This links statically to the libIDL version in the repository, avoiding
# conflicts with other versions.
LIBS=../libIDL/.libs/libIDL.a `glib-config --libs` \
     -L $(BUILDROOT)/KL/String/binaries -lstring

OFLAGS= -o

all:$(FINAL)

objects/%.o: %.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) -o $@ $(FLAGS) $<

$(FINAL): $(OBJS)
	$(CC) $(OBJS) $(LIBS) $(OFLAGS) $@

clean:
	$(RM) $(OBJS) $(FINAL) *~ binaries/*.c binaries/*.h
