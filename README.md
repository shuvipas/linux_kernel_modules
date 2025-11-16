# linux_kernel_modules
This repository contains a collection of Linux kernel modules written for self-learning purposes.
Each folder demonstrates a specific subsystem or API of the Linux kernel, including character devices, /proc filesystem integration, PCI drivers, and core kernel mechanisms such as memory allocation, spinlocks, and kernel threads.

The modules are designed to be simple, self-contained examples that can be built and loaded individually.

## Installing Required Tools

Linux distributions provide the commands modprobe, insmod, and lsmod inside the kmod package.

On Fedora:
```bash
sudo dnf install kmod
```

On Ubuntu/Debian:
```bash 
sudo apt-get install build-essential kmod
```
On Arch Linux:
```bash
sudo pacman -S gcc kmod
```
## Install Kernel Headers
Before building anything, it is necessary to install the header files for the kernel.

On Fedora:
```bash
sudo dnf install kernel-devel kernel-headers
```

On Ubuntu/Debian:
```bash
sudo apt-get update 
apt-cache search linux-headers-`uname -r`
sudo apt-get install linux-headers-`uname -r`
```

On Arch Linux:
```bash
sudo pacman -S linux-headers
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
Device specification: https://www.qemu.org/docs/master/specs/edu.html

Demonstrates:

    PCI device discovery and initialization

    BAR mapping with pci_iomap

    MMIO register interaction

    Interrupt handling (INTx / MSI)

    DMA transfers using the device’s 4 KB internal buffer

    IOCTL interface to control the device from user space

    User-space test application (edu_test.c)

Features Exercised:

    Device ID read and liveness check

    Factorial computation unit

    Interrupt raise/ack

    DMA host-to-device and device-to-host operations

### Kernel API Modules — kernel_api_modules

A collection of standalone small modules demonstrating fundamental kernel APIs.

#### Concepts Across modules:

    Module parameters

    Kernel threads & wait queues

    Spinlocks / atomic context

    task_struct traversal

    Virtual memory areas (vm_area_struct)

    Kernel logging (pr_info, pr_debug)

    Dynamic allocation with kmalloc/kfree and process metadata tracking.

    Process and VMA iteration using kernel APIs and version-dependent macros.
