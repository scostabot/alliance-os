######################################################################
# Alliance Cache Kernel Makefile
#
# HISTORY
# Date     Author    Rev    Notes
# 14/11/98 ramon     1.0    First version

CC= gcc
CFLAGS= -Wall -W -g
CPPFLAGS= -DSPARC -DSKTRACE
PLATFORM= SPARC
RM= rm -f

######################################################################
# User-definable Settings
#

# DEBUGGER has three possibilities:
# - GDB-NOHLT links in the remote GDB stubs but doesn't halt on startup
# - GDB links in the remote GDB stubs and executes a breakpoint
#   during CK startup, so that the debugging session can be started
# - anything else links in the old CK debugger
#
DEBUGGER    = GDB-NOHLT

# Command line options
PPFLAGS     = -I./include -I../include -I./lib/include
CFLAGS2     = -O3 -fomit-frame-pointer -m486
LDFLAGS     = -Wl,-Ttext,0xc0150000 -nostartfiles -nostdlib \
              -L../KL/String/binaries -L../KL/Alloc/binaries \
              -L../KL/Memory/binaries

######################################################################
# End of settings
#

# Object files
OBJS	= objects/entry.o objects/pri_heap.o objects/sched.o \
          objects/set_vec.o objects/init.o objects/text.o objects/tasks.o \
          objects/jump.o objects/idle.o objects/memory.o objects/memhw.o \
          objects/graph.o objects/objmgmt.o objects/paging.o \
          objects/callgates.o objects/cachekthread.o objects/kernel.o \
          objects/exceptions.o objects/resource.o objects/math.o \
          objects/irq.o objects/signal.o objects/main.o objects/shmem.o
GDBDBG	= objects/gdb-stub.o objects/dbg-par.o
CKDBG	= objects/debugger.o

# Libraries
LIBS    = -lstring -lmemory -lalloc

# The kernel filename
FINAL = ./binaries/CK

# Link in the correct debugger
ifeq ($(DEBUGGER),GDB-NOHLT)
	OBJS	+= $(GDBDBG)
else
ifeq ($(DEBUGGER),GDB)
	OBJS	+= $(GDBDBG)
	PPFLAGS += -DDEBUG
else
	OBJS	+= $(CKDBG)
endif
endif

######################################################################
# Make rules
#

# Do all

all: $(FINAL)
	cd demo && gmake

# Link the kernel

$(FINAL): $(OBJS) ./binaries/libck.a
	$(CC)  -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

./binaries/libck.a:
	cd lib && gmake

# Compile the source files
objects/%.o: %.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: architecture/$(PLATFORM)/%.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: lib/architecture/$(PLATFORM)/%.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: monitor/%.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: demo/%.c
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: %.S
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: architecture/$(PLATFORM)/%.S
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: lib/architecture/$(PLATFORM)/%.S
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

objects/%.o: monitor/%.S
	$(CC)  $(CPPFLAGS) $(PPFLAGS) $(CFLAGS) $(CFLAGS2) -c -o $@ $<


# Clean up the junk
clean:
	$(RM) $(OBJS) $(FINAL) $(GDBDBG) $(CKDBG)
	cd demo && gmake clean
	cd lib && gmake clean

######################################################################
# The End
#
