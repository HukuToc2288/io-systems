ccflags-y := -std=gnu99
obj-m += lab1.o

all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules
clean:
	make -C /lib/modules/`uname -r`/build /usr/include M=$(PWD) clean

