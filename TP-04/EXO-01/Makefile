KERNELDIR_PNL ?= ../../linux-4.19
PWD := $(shell pwd)

obj-m += helloWorld.o
obj-m += helloWorldParam.o

all :
	make -C $(KERNELDIR_PNL) M=$(PWD) modules

clean:
	make -C $(KERNELDIR_PNL) M=$(PWD) clean

