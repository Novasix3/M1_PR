.PHONY: all 

KERNELDIR_PNL ?= ../../linux-4.19
PWD := $(shell pwd)

obj-m += kcpustat.o

all :
	$(info obj-m : $(obj-m))
	make -C $(KERNELDIR_PNL) M=$(PWD) modules

clean:
	make -C $(KERNELDIR_PNL) M=$(PWD) clean
