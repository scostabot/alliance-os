######################################################################
# Alliance Third-Stage Bootloader (ABL) Makefile
#
# HISTORY
# Date     Author    Rev    Notes
# 14/11/98 ramon     1.0    First version
# 15/10/99 scosta    1.1    Added autoconf stuff and renamed to Makefile.in
# 16/02/00 scosta    1.2    Added PLATFORM autoconf variable

CC=gcc
CFLAGS=-Wall -W -g
CPPFLAGS=-DSPARC -DSKTRACE
RM=rm -f
PLATFORM=SPARC

######################################################################
# User-definable Settings
#

PPFLAGS     = -I./include -I../../include -I../../CK/include
CFLAGS2     = -O3 -fomit-frame-pointer -m486
LDFLAGS     = -Wl,-T,scripts/ABL.ld -nostartfiles -nostdlib \
              -L../../KL/String/binaries -L../../KL/Memory/binaries \
              -L../shared/binaries

######################################################################
# End of settings
#

OBJS	= objects/entry.o objects/main.o objects/text.o \
          objects/intr.o objects/elf.o objects/delay.o \
          objects/gdt.o objects/memory.o objects/smp.o \
          objects/script.o

LIBS    = -lstring -lmemory -lsharedboot

FINAL   = ./binaries/ABL

######################################################################
# Make rules
#

# Do all

all: $(FINAL)

# Link the kernel

$(FINAL): $(OBJS) ../shared/binaries/libsharedboot.a scripts/ABL.ld
	$(CC)  -o $@ $(OBJS) $(LIBS) $(LDFLAGS)


# Compile the source files

objects/%.o: %.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: architecture/$(PLATFORM)/%.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: %.S
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: architecture/$(PLATFORM)/%.S
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<


# Clean up the junk

clean:
	$(RM) $(OBJS) $(FINAL)

######################################################################
# The End
#
