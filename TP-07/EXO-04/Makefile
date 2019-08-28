.PHONY: all 

KERNELDIR_PNL ?= ../../linux-4.19.3
PWD := $(shell pwd)

obj-m += taskmonitormempool.o

all :
	$(info obj-m : $(obj-m))
	make -C $(KERNELDIR_PNL) M=$(PWD) modules

clean:
	make -C $(KERNELDIR_PNL) M=$(PWD) clean
