# Makefile for the Alliance DOS Bootloader
#
# HISTORY
# Date     Author    Rev    Notes
# 14/11/98 ramon     1.0    First release
# 04/05/04 scosta    1.1    Modularized with config install

MAKE=gmake
RM=rm -f
NASM=nasm

LOADERSRC = loader.asm
LOADER    = ./binaries/loader.com

all : $(LOADER)
	cd test/ && $(MAKE) -f MakeTest

$(LOADER) : $(LOADERSRC)
	$(NASM)  -f bin -o $(LOADER) $(LOADERSRC)

clean:
	$(REMOVE) $(LOADER)
	cd test/ && $(MAKE) clean
