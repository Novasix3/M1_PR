obj-m += one_addr.o

all:
	make -C ../../linux-4.19/ M=$(PWD) modules

clean:
	make -C ../../linux-4.19/ M=$(PWD) clean
