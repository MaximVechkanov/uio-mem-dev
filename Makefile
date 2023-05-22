obj-m = uio_mem_dev.o

KVERSION = $(shell uname -r)
all:
	make -C ~/workspace/WSL2-Linux-Kernel M=$(PWD) modules

clean:
	make -C ~/workspace/WSL2-Linux-Kernel M=$(PWD) clean