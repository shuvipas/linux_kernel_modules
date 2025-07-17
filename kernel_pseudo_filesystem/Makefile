obj-m += hello_sysfs.o
obj-m += procfs_4.o
obj-m += procfs_3.o
obj-m += procfs_2.o
obj-m += procfs_1.o


startstop-objs := start.o stop.o
PWD := $(CURDIR)

all:
		$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
		$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean