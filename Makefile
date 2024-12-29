obj-m += ioctl_module.o

all:
	make -C /usr/src/linux-$(shell uname -r) SUBDIRS=$(PWD) modules
	gcc -o keyboard_test keyboard_test.c
clean:
	rm -f .test_module.ko.cmd .test_module.mod.o.cmd .test_module.o.cmd Module.symvers modules.order test_module.ko test_module.mod.o test_module.o .ioctl_module.ko.cmd .ioctl_module.mod.o.cmd .ioctl_module.o.cmd ioctl_module.ko ioctl_module.mod.o ioctl_module.o ioctl_module.mod.c test_module.mod.c ioctl_test keyboard_test