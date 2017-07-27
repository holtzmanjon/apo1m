obj-m += astropci.o

default:
	make -C /lib/modules/`uname -r`/build/ SUBDIRS=`pwd` modules
	make -C /lib/modules/`uname -r`/build/ SUBDIRS=`pwd` modules_install

# Could have done it this way too.
#	make -C /lib/modules/$(shell uname -r)/build/ SUBDIRS=$(PWD) modules
#	make -C /lib/modules/$(shell uname -r)/build/ SUBDIRS=$(PWD) modules_install

