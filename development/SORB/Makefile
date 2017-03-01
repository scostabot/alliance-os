GCC            = $(CC)
DEBUG          = -g
WARN           = -Wall
LOCAL_INCLUDE  = ./include/
GLOBAL_INCLUDE = ../include/
INCLUDE        = -I$(LOCAL_INCLUDE) -I$(GLOBAL_INCLUDE)
COMPILE        = $(GCC) $(DEBUG) $(WARN) $(INCLUDE) $(CPPFLAGS)
OBJDIR         = ./objects
BINDIR         = ./binaries
BINARIES       =  binaries/libEcho.so binaries/libEcho-na.so binaries/libOrb.so binaries/libEcho-stub.so binaries/client binaries/prueba


all: binaries/libEcho.so binaries/libEcho-na.so binaries/libOrb.so binaries/libEcho-stub.so binaries/client binaries/prueba
#	cat scripts/finished.msg

objects/implementation.o: implementation.c
	$(COMPILE) -c -fPIC -o $(OBJDIR)/implementation.o $<

binaries/libEcho.so: objects/implementation.o
	$(COMPILE) -shared -o $(BINDIR)/libEcho.so $<

objects/implementation-na.o: implementation-na.c
	$(COMPILE) -c -fPIC -o $(OBJDIR)/implementation-na.o $<

binaries/libEcho-na.so: objects/implementation-na.o
	$(COMPILE) -shared -o $(BINDIR)/libEcho-na.so $<

objects/orb.o : orb.c
	$(COMPILE) -c -fPIC -o $(OBJDIR)/orb.o $<

binaries/libOrb.so : objects/orb.o
	$(COMPILE) -shared -o $(BINDIR)/libOrb.so $<

objects/echo-stub.o : echo-stub.c
	$(COMPILE) -fPIC -c -o $(OBJDIR)/echo-stub.o $<

binaries/libEcho-stub.so : objects/echo-stub.o binaries/libOrb.so
	$(COMPILE) -shared -nostartfiles -o $(BINDIR)/libEcho-stub.so $(OBJDIR)/echo-stub.o -L$(BINDIR) -ldl -lOrb

objects/client.o : client.c
	$(COMPILE) -c -o $(OBJDIR)/client.o $<

binaries/client : objects/client.o binaries/libEcho-stub.so
	$(COMPILE) -o $(BINDIR)/client $(OBJDIR)/client.o -L$(BINDIR) -lEcho-stub -Xlinker -rpath -Xlinker $(BINDIR)

objects/prueba.o : prueba.c
	$(COMPILE) -c -o $(OBJDIR)/prueba.o $<

binaries/prueba : objects/prueba.o
	$(COMPILE) -o $(BINDIR)/prueba -L$(BINDIR) -lOrb $<

clean:
	rm -f objects/*.o *~ include/*~ ../include/*~ core $(BINARIES)
