obj-m += poison.o

all:
	make -C ../../linux-4.19/ M=$(PWD) modules

clean:
	make -C ../../linux-4.19/ M=$(PWD) clean
