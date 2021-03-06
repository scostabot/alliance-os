# Top-level makefile for Alliance OS generation.
# This makefile will trigger the generation of the whole OS, plus managing
# system-wide options, like test program generation.
#
# HISTORY
# Date     Author    Rev    Notes
# 15/10/99 scosta    1.0    First draft
# 16/02/00 scosta    1.1    Added ABL in BOOTMODULES

MAKE=gmake

KLMODULES=development/KL/Alloc development/KL/Memory development/KL/Msg \
          development/KL/String development/KL/Monitor development/KL/Queue \
          development/KL/Exec development/KL/Threads

KLMODULES_WITHTEST=development/KL/Memory development/KL/String development/KL/Monitor \
                   development/KL/Queue development/KL/Exec/TestSuite development/KL/Exec

CKMODULES=development/CK

BOOTMODULES=development/boot/GRUB development/boot/ABL/

LMMODULES=development/LM
LMMODULES_WITHTEST=development/LM/TestSuite development/LM

IOSKMODULES=development/IOSK

LMIOSKMODULES=development/LM/IOSK/ibm-pc/ParallelPort

all:
	for DIR in $(KLMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(KLMODULES_WITHTEST); do (cd $$DIR ; $(MAKE) -f MakeTest) ; done
	for DIR in $(LMMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(LMMODULES_WITHTEST) ; do (cd $$DIR ; $(MAKE) -f MakeTest) ; done
	for DIR in $(LMIOSKMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(CKMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(BOOTMODULES) ; do (cd $$DIR ; $(MAKE)) ; done

kl:
	for DIR in $(KLMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(KLMODULES_WITHTEST); do (cd $$DIR ; $(MAKE) -f MakeTest) ; done

lm:
	for DIR in $(LMMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(LMMODULES_WITHTEST) ; do (cd $$DIR ; $(MAKE) -f MakeTest) ; done

test:
	for DIR in $(KLMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(KLMODULES_WITHTEST); do (cd $$DIR ; $(MAKE) -f MakeTest) ; done
	for DIR in $(KLMODULES_WITHTEST); do (cd $$DIR ; $(MAKE) -f MakeTest test) ; done
	for DIR in $(LMMODULES_WITHTEST) ; do (cd $$DIR ; $(MAKE) -f MakeTest) ; done
	for DIR in $(LMMODULES_WITHTEST) ; do (cd $$DIR ; $(MAKE) -f MakeTest test) ; done

allbuttests:
	for DIR in $(KLMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(LMMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(CKMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(BOOTMODULES) ; do (cd $$DIR ; $(MAKE)) ; done
	for DIR in $(IOSKMODULES) ; do (cd $$DIR ; $(MAKE)) ; done

install:

clean:
	for DIR in $(KLMODULES) ; do (cd $$DIR ; $(MAKE) clean) ; done
	for DIR in $(KLMODULES_WITHTEST); do (cd $$DIR ; $(MAKE) -f MakeTest clean) ; done
	for DIR in $(BOOTMODULES) ; do (cd $$DIR ; $(MAKE) clean) ; done
	for DIR in $(CKMODULES) ; do (cd $$DIR ; $(MAKE) clean) ; done
	for DIR in $(IOSKMODULES) ; do (cd $$DIR ; $(MAKE) clean) ; done
	for DIR in $(LMMODULES) ; do (cd $$DIR ; $(MAKE) clean) ; done
	for DIR in $(LMMODULES_WITHTEST) ; do (cd $$DIR ; $(MAKE) -f MakeTest clean) ; done
   

