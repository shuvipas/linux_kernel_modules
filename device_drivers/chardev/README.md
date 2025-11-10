# Character Device Driver Examples (chardev)  
This folder contains **three example Linux kernel modules** demonstrating the evolution of a character‑device driver: from a simple read‑only device, to exclusive access with IOCTL support, to a more “modern” implementation using `alloc_chrdev_region()` and `cdev`.  

These modules serve both as educational material (for your embedded/SoC/driver development interests) and as practical code you can discuss in interviews (e.g., for systems, Linux kernel, driver, C) or use as a base for further experimentation.  

---

## Table of Contents  
1. Introduction  
2. Prerequisites  
3. Build & Usage (common instructions)  
4. Overview of the Modules  
   - 4.1 `chardev.c` — simple read‑only char device  
   - 4.2 `char_dev` module — I/O char device + IOCTL support  
   - 4.3 `ioctltest` module — alloc_chrdev_region + cdev + multiple IOCTLs  
5. User‑Space Test Application (for the IOCTL module)  
6. What You’ll Learn / Key Concepts  
7. Limitations & Caveats  
8. Contributing  
9. License  

---

## 1. Introduction  
Character devices provide a byte‑oriented interface between user space and kernel space, through special device nodes (e.g. `/dev/<name>`). These examples illustrate how to:  
- register a char device (major/minor) and expose a device node;  
- implement `open`, `read`, `write`, `release` (and optionally `ioctl`) operations via the `struct file_operations` interface;  
- interact with user space via `copy_to_user()/get_user()/put_user()`;  
- manage device lifetime (module_init, module_exit, device_create, class_create, unregister, etc).  
These topics are well‑documented in resources such as *The Linux Kernel Module Programming Guide*. :contentReference[oaicite:0]{index=0}  

---

## 2. Prerequisites  
- A Linux system or VM with kernel headers installed (must match the version of your running kernel).  
- Root privileges to load/unload modules and to create device nodes (if needed).  
- Basic knowledge of building kernel modules (Makefile/Kbuild), using `insmod`/`rmmod`, reading `dmesg`.  
- Familiarity with C, pointers, kernel vs user space, and Linux file operations.  

---

## 3. Build & Usage  
### Build  
```bash
cd device_drivers/chardev
make
sudo insmod <module_name>.ko
dmesg | tail ‑n20   # check for major number/device creation message
```

