KERNELDIR_PNL ?= ../../linux-4.19
PWD := $(shell pwd)

obj-m += show_sb.o
obj-m += update_sb.o

all :
	make -C $(KERNELDIR_PNL) M=$(PWD) modules

clean:
	make -C $(KERNELDIR_PNL) M=$(PWD) clean

