GCC=gcc
DEBUG=-g
WARN=-Wall
PLATFORM=-DI80X86
LOCAL_INCLUDE=./include
GLOBAL_INCLUDE=../../include
INCLUDE=-I$(LOCAL_INCLUDE) -I$(GLOBAL_INCLUDE)
COMPILE=$(GCC) $(DEBUG) $(WARN) $(INCLUDE) $(PLATFORM)
OBJDIR=./objects
BINDIR=./binaries

default : all

objects/libIPC.o: libIPC.c
	$(COMPILE) -c -fPIC \
		-o $(OBJDIR)/libIPC.o \
		libIPC.c
objects/ipcqueue.o: ipcqueue.c
	$(COMPILE) -c -fPIC \
		-o $(OBJDIR)/ipcqueue.o \
		ipcqueue.c
binaries/libIPC.so: objects/libIPC.o objects/ipcqueue.o
	$(COMPILE) -shared -nostartfiles \
		-o $(BINDIR)/libIPC.so \
		$(OBJDIR)/libIPC.o \
		$(OBJDIR)/ipcqueue.o


objects/exec1.o: exec1.c
	$(COMPILE) -c \
		-o $(OBJDIR)/exec1.o \
		exec1.c
binaries/exec1: objects/exec1.o
	$(COMPILE) \
		-o $(BINDIR)/exec1 \
		$(OBJDIR)/exec1.o \
		-L$(BINDIR) -lIPC \
		-Xlinker -rpath -Xlinker $(BINDIR)
objects/exec2.o: exec2.c
	$(COMPILE) -c \
		-o $(OBJDIR)/exec2.o \
		exec2.c
binaries/exec2: objects/exec2.o
	$(COMPILE) \
		-o $(BINDIR)/exec2 \
		$(OBJDIR)/exec2.o \
		-L$(BINDIR) -lIPC \
		-Xlinker -rpath -Xlinker $(BINDIR)

objects/clean.o: clean.c
	$(COMPILE) -c \
		-o $(OBJDIR)/clean.o \
		clean.c
binaries/clear: objects/clean.o
	$(COMPILE) \
		-o $(BINDIR)/clear \
		$(OBJDIR)/clean.o

all: binaries/libIPC.so binaries/exec1 binaries/exec2 binaries/clear
	cat scripts/finished.msg

clean:
	rm -f -r $(OBJDIR)/*.o $(BINDIR)/* *~ core
