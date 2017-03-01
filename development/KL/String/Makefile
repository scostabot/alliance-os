# Compile the Alliance Kernel Library
#
# HISTORY
# Date     Author    Rev    Notes
# 22/10/98 scosta    1.0    First draft
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
PPFLAGS= -I ../../include

# This is generic to all modules, so maybe defined externally, but in which way?
# CPPFLAGS will be set outside, so we guarantee omogeneous compile

objects/%.o: %.c $(COMMONDEP) 
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

# This is what we need to compile in this module

SRCS= StringAppend.c StringCompare.c StringCompareNoCase.c StringCopy.c \
      StringLength.c StringMatch.c StringNumCopy.c StringFormat.c

OBJS= objects/StringAppend.o objects/StringCompare.o \
      objects/StringCompareNoCase.o objects/StringCopy.o \
      objects/StringLength.o objects/StringMatch.o objects/StringNumCopy.o \
      objects/StringFormat.o

FINAL= ./binaries/libstring.a

all: $(FINAL)

$(FINAL): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(FINAL) *~

