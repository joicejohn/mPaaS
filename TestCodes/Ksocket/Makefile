.PHONY: all clean

obj-m := ksockTest.o
ksockTest-objs := ksockTest.o ksocket.o

EXTRA_CFLAGS := -O2 
KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.ko *.o *.mod.* .*.cmd *.c~ .tmp_versions Module.symvers

