######################################################################
# Alliance Cache Kernel Demo System Makefile
#
# HISTORY
# Date     Author    Rev    Notes
# 14/11/98 ramon     1.0    First version


######################################################################
# User-definable Settings
#

CC=gcc
CFLAGS=-Wall -W -g
CPPFLAGS=-DSPARC -DSKTRACE
RM=rm -f

PPFLAGS     = -I../../include -I../include -I../lib/include -I./
CFLAGS2     = -O3 -fomit-frame-pointer -m486
LDFLAGS     = -Wl,-Ttext,0x2000 -nostartfiles -nostdlib \
              -L../../KL/String/binaries -L../binaries

######################################################################
# End of settings
#

OBJS	= ../objects/inittask.o ../objects/demotasks.o \
          ../objects/demokernel.o ../objects/demotext.o

LIBS    = -lck -lstring -lgcc

# The kernel filename
FINAL = ../binaries/DEMO

######################################################################
# Make rules
#

# Do all

all: $(FINAL)

# Link the kernel

$(FINAL): $(OBJS)
	$(CC)  -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

# Compile the source files
../objects/%.o: %.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

# Clean up the junk
clean:
	$(RM) $(OBJS) $(FINAL)

######################################################################
# The End
#
