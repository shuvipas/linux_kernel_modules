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