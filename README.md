# linux_kernel_modules
This repository contains a collection of Linux kernel modules written for self-learning purposes.
Each folder demonstrates a specific subsystem or API of the Linux kernel, including character devices, /proc filesystem integration, PCI drivers, and core kernel mechanisms such as memory allocation, spinlocks, and kernel threads.

The modules are designed to be simple, self-contained examples that can be built and loaded individually.
to bulid them you need to install some required tools header files for the kernel.

On Fedora:
```bash
sudo dnf install kmod kernel-devel kernel-headers
```

On Ubuntu/Debian:
```bash 
sudo apt-get install build-essential kmod
sudo apt-get update 
apt-cache search linux-headers-`uname -r`
sudo apt-get install linux-headers-`uname -r`
```

## How to build & run the modules 
```bash
cd <example_folder>
make
sudo insmod example_module.ko
dmesg | tail -n 10
sudo rmmod example_module
dmesg | tail -n 10
make clean
```

## Project Structure

Below is an overview of each module group included in this repository.

### Keylogger 

    This folder contains a Linux kernel module that implements a simple keyboard logger using a character device interface. The module captures key presses from the PS/2 keyboard and stores them in a buffer accessible from user space.

### Character Device Drivers — device_drivers/chardev

    Examples demonstrating creation of character devices and interaction through /dev.

    Concepts:

    register_chrdev, cdev, file_operations

    Implementing .open, .read, .write, .unlocked_ioctl

    User-space interaction through custom IOCTL commands

    Basic kernel–userspace communication patterns

### Pseudo Filesystems — kernel_pseudo_filesystem

    Modules that create custom entries under /proc.

    Includes:

    Simple read/write /proc file with manual buffering and permissions

    Sequential output using the seq_file interface (seq_read, seq_printf)

    Examples of offset handling, memory copying, and safe user-space access

    Concepts:

    proc_create, proc_ops / file_operations

    copy_to_user, copy_from_user

    Sequential iteration using seq_operations

### PCI Driver for QEMU EDU Device — pci_driver/edu

A full PCI driver implementation for the QEMU EDU virtual device.
### Netfilter module
The netfilter module monitors outbound TCP connections by implementing a netfilter hook on the NF_INET_LOCAL_OUT chain. It initially detects and logs TCP connection initiation packets (SYN flag set, ACK flag cleared) by displaying their source IP address and port. The module supports destination based filtering through an ioctl interface, allowing users to specify a target IP address only packets destined for that address will be logged while others are ignored. The module handles network byte order conversion for ports and uses kernel-appropriate functions for IP address comparison and user-space data transfer.
#### Bulid procces
run test_runner.sh 

### Kernel API Modules — kernel_api_modules

A collection of standalone small modules demonstrating fundamental kernel APIs.

