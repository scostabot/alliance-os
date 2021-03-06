###########################################################################
# Compile the Alliance Boot System shared library
#
# HISTORY
# Date       Author    Rev    Notes
# 23/05/99   Ramon     1.0    First version

CC=gcc
CFLAGS=-Wall -W -g
CPPFLAGS=-DSPARC -DSKTRACE
AR=ar
ARFLAGS=cruv
RM=rm -f

##########################################################################
# User-definable settings
#

# select the ethernet drivers you want (or none)
NETFLAGS += -DINCLUDE_NE
NETFLAGS += -DINCLUDE_NEPCI
NETFLAGS += -DINCLUDE_WD
NETFLAGS += -DINCLUDE_T503
NETFLAGS += -DINCLUDE_T509
NETFLAGS += -DINCLUDE_NE2100
NETFLAGS += -DINCLUDE_CS
NETFLAGS += -DINCLUDE_EEPRO100

# if you uncomment this, the configuration file will be expected
# to have the low byte of the requestor IP address as an extension
#NETFLAGS += -DCONFIG_FILE_EXT

# add some extra flags for drivers
EXTRA_NETFLAGS += -DNE_SCAN=0x280,0x300,0x320,0x340 # req'd by INCLUDE_NE!
EXTRA_NETFLAGS += -DWD_DEFAULT_MEM=0xCC000 # req'd by INCLUDE_WD!
#EXTRA_NETFLAGS += CS_SCAN=list...
#EXTRA_NETFLAGS += -D_3COM_USE_AUI # Disable transceiver on 3c503 by default

FLAGS   = -o $@ -c -fno-builtin -nostdinc -O2
PPFLAGS = -I../../include -I./include

##########################################################################
# End of settings
#

OBJS  = objects/disk_io.o objects/fsys_ext2.o objects/fsys_fat.o \
        objects/fsys_ffs.o objects/gunzip.o objects/misc.o \
        objects/biosdisk.o objects/biostext.o objects/i86.o \

ifdef NETFLAGS
PPFLAGS += -DFSYS_TFTP
OBJS += objects/fsys_tftp.o objects/config.o objects/ip.o objects/pci.o
OBJS += objects/3c509.o objects/cs89x0.o objects/eepro100.o \
        objects/lance.o objects/ns8390.o
endif

FINAL = binaries/libsharedboot.a

##########################################################################

objects/%.o: %.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

objects/%.o: fs/%.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

objects/%.o: net/%.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(NETFLAGS) $(FLAGS) $<

objects/%.o: drivers/%.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

objects/%.o: drivers/net/%.c
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(NETFLAGS) $(EXTRA_NETFLAGS) $(FLAGS) $<

objects/%.o: drivers/%.S
	$(CC) $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(FLAGS) $<

all: $(FINAL)

$(FINAL): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(RM) -f $(OBJS) $(FINAL) *~

