# Generation of the Dynamic Linker shell makefile
#
# HISTORY
# Date     Author    Rev    Notes
# 05/03/00 scosta    1.0    First draft

MAKE=gmake
RM=rm -f

DYNALINK=elfloader/ shell/ 
SAMPLES=samplecode/

all:
	for DIR in $(DYNALINK) ; do (cd $$DIR ; $(MAKE)) ; done

clean:
	$(RM) *~
	for DIR in $(DYNALINK) ; do (cd $$DIR ; $(MAKE) clean) ; done
	for DIR in $(SAMPLES) ; do (cd $$DIR ; $(MAKE) -f MakeTest clean) ; done

test:
	for DIR in $(SAMPLES) ; do (cd $$DIR ; $(MAKE) -f MakeTest) ; done


