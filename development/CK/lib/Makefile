######################################################################
# Alliance Cache Kernel Library Makefile
#
# HISTORY
# Date     Author    Rev    Notes
# 14/11/98 ramon     1.0    First version

CC=gcc
CFLAGS=-Wall -W -g
CPPFLAGS=-DSPARC -DSKTRACE
PLATFORM=SPARC
AR=ar
ARFLAGS=cruv
RM=rm -f

######################################################################
# User-definable Settings
#

PPFLAGS     = -I../include -I../../include -I./include
CFLAGS2     = -O3 -fomit-frame-pointer -m486

######################################################################
# End of settings
#

OBJS	= ../objects/cachethread.o ../objects/syscalls.o

# The kernel filename
FINAL = ../binaries/libck.a

######################################################################
# Make rules
#

# Do all

all: $(FINAL)

# Link the kernel

$(FINAL): $(OBJS)
	$(AR)  $(ARFLAGS) $@ $(OBJS)

# Compile the source files
../objects/%.o: architecture/$(PLATFORM)/%.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

# Clean up the junk
clean:
	$(RM) $(OBJS) $(FINAL)

######################################################################
# The End
#
