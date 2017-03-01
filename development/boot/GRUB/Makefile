######################################################################
# Grand Unified Bootloader (GRUB) Makefile
#
# HISTORY
# Date     Author    Rev    Notes
# 23/05/99 ramon     1.0    First version

MAKE=gmake
RM=rm -f

######################################################################
# User-definable Settings
#

export OBJCOPY      = objcopy
export SHARED_FLAGS = -fno-builtin -nostdinc -O2

######################################################################
# End of settings
#

PROGS=	stage1 stage2

all:
	@for X in $(PROGS) ; do (cd $$X ; $(MAKE)) ; done

clean:
	@for X in $(PROGS) ; do (cd $$X ; $(MAKE) clean) ; done
	$(RM) config.log
	cd binaries && $(RM) $(PROGS)
