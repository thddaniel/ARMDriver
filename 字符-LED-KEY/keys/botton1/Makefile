KERNELDIR=/home/passion/share/transplant/linux-2.6.30.4

PWD:=$(shell pwd)

CROSS_COMPILE=arm-linux-
CC=$(CROSS_COMPILE)gcc

obj-m:=plat_buttons.o plat_device.o

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.cmd *.ko *.mod.c *.order *.symvers .bak
