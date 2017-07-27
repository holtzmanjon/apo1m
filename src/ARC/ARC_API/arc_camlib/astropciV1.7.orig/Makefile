
.SUFFIXES: _g.o

CC	= gcc -m486 -pipe -I /usr/include
LD	= ld
MAKE	= make

CFLAGS	= -Wall
OFLAGS  = -O2
UFLAGS	= -D__CONFIG__
#GFLAGS	= -g -DDEBUG_ASTROPCI 

OS_DEP  = "system_is_unknown_(run_make_config)"
OS_DEP2 = "system_is_unknown_(run_make_config)"
-include .system

M_SRCS	= $(OS_DEP).c
M_OBJS	= $(OS_DEP).o
MGOBJS	= $(OS_DEP)_g.o

TARGET	= astropci.o
TARG_G	= astropci_g.o

all:		config module debug 

module:		$(TARGET)

$(TARGET):	$(M_OBJS)
		$(LD) -r -o $@ $(M_OBJS) 

debug:		$(TARG_G)

$(TARG_G):	$(MGOBJS)
		$(LD) -r -o $@ $(MGOBJS)

.c.o:
		$(CC) $(CFLAGS) $(OFLAGS) $(KFLAGS) \
		-o $@ -c $<

.c_g.o:
		$(CC) $(CFLAGS) $(OFLAGS) $(KFLAGS) \
		$(GFLAGS) -o $@ -c $<

install:
		RELEASE=`uname -r`; \
		cp astropci_g.o /lib/modules/$$RELEASE/kernel/drivers/astropci/. ;\
		cp astropci.o /lib/modules/$$RELEASE/kernel/drivers/astropci/. ; \
		chmod 644  /lib/modules/$$RELEASE/kernel/drivers/astropci/* 
clean:
		-rm -f *.o 

config:
		@SYSTEM=`uname -s`; \
		RELEASE=`uname -r`; \
		case $$SYSTEM in \
		Linux) \
		  echo "KFLAGS=-fomit-frame-pointer -D__KERNEL__ -DMODULE" > \
		  .system; \
		  IFS='.'; set $$RELEASE; \
		  if [ $$1 -lt "2" ]; then \
		    echo "$$SYSTEM $$RELEASE IS NOT SUPPORTED"; \
		    rm .system; exit 1; \
		  else \
		    case $$2 in \
		    4) echo "OS_DEP=astropci_main" >> .system; \
		    ;; \
		    *) echo "$$SYSTEM $$RELEASE IS NOT SUPPORTED"; \
		       rm .system; exit 1 \
		    ;; \
		    esac; \
		  fi \
		;; \
		*) echo "$$SYSTEM $$RELEASE IS NOT SUPPORTED"; \
		   rm .system; exit 1 \
		;; \
		esac; \
		echo "Building for $$SYSTEM $$RELEASE";


