# obj-m += hello_1.o see c file
obj-m += hello_2.o
obj-m += hello_3.o
obj-m += hello_4.o
obj-m += hello_5.o
obj-m += startstop.o
startstop-objs := start.o stop.o
PWD := $(CURDIR)

all:
		$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
		$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean