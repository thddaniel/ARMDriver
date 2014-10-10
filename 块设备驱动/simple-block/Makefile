ifneq ($(KERNELRELEASE),)

obj-m := simple-blk.o

else

KDIR := /lib/modules/2.6.18-53.el5/build
all:
	make -C $(KDIR) M=$(PWD) modules 
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers

endif
