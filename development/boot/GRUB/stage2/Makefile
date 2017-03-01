###########################################################################
#  GRUB stage2 makefile
#

CC=gcc
CPPFLAGS=-DSPARC -DSKTRACE
CFLAGS= $(SHARED_FLAGS)
OBJCOPY=objcopy
LD=/usr/ccs/bin/ld
RM=rm -f

PPFLAGS= $(SHARED_FLAGS) -I../include -I../../../include

# "asm.o" absolutely must come first!!!
SHARED_OBJS=	../objects/asm.o ../objects/common.o ../objects/char_io.o \
                ../objects/boot.o ../objects/cmdline.o ../objects/stage2.o

all:	../binaries/stage2

../objects/%.o: %.S
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) -o $@ -c $<

../objects/%.o: %.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) -o $@ -c $<

../objects/stage2.elf:	$(SHARED_OBJS) stage2.ld ../../shared/binaries/libsharedboot.a
	$(LD) -N -T stage2.ld -o ../objects/stage2.elf $(SHARED_OBJS) ../../shared/binaries/libsharedboot.a

../binaries/stage2:	../objects/stage2.elf
	$(OBJCOPY) -O binary ../objects/stage2.elf ../binaries/stage2

clean:
	$(RM) $(SHARED_OBJS) ../objects/stage2.elf

