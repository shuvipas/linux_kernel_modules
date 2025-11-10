# Kernel Pseudo‑Filesystem Examples  
This folder contains Linux kernel modules that demonstrate how to create entries under the pseudo‑filesystem (`/proc`), showing a progression from read‑only, to read/write, to an enhanced buffer version.

## Included Modules  
1. Read‑only proc file (`/proc/helloworld`)  
2. Read/write proc file (`/proc/buffer1k`)  
3. Enhanced read/write proc file (`/proc/buffer2k`, 2 KB buffer and module reference management)  

---

## Overview  
### Version 1 – Read‑only (`/proc/helloworld`)  
- Creates a file named `helloworld` under `/proc` (via `proc_create()` or older API).  
- Provides a read handler returning a static message (“HelloWorld!\n”) using `copy_to_user()`.  
- Does *not* support writes.  
- Adapts to kernel version: uses `struct proc_ops` if `LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)` or falls back to `struct file_operations`.  
**Key learning:** basic proc‑filesystem integration and user‑space read from kernel module.  

### Version 2 – Read/Write (`/proc/buffer1k`)  
- Creates `buffer1k` under `/proc`.  
- Provides both `.read` and `.write` handlers:  
  - `.write`: copies data from user space via `copy_from_user()` into a static kernel buffer of size 1 KB.  
  - `.read`: copies data from kernel buffer to user space.  
- Uses `PROCFS_MAX_SIZE = 1024` to bound buffer size.  
- Demonstrates how a user can `echo “…” | sudo tee /proc/buffer1k` to write, then `cat /proc/buffer1k` to read.  
**Key learning:** enabling both directions of communication via procfs, buffer management.  

### Version 3 – Enhanced Buffer & Module Reference (`/proc/buffer2k`)  
- Creates `buffer2k` under `/proc`.  
- Provides read/write handlers similar to version 2, but:  
  - Uses `PROCFS_MAX_SIZE = 2048` (2 KB buffer).  
  - Uses `min()` macro to clamp length to buffer size (when kernel version supports). :contentReference[oaicite:0]{index=0}  
  - Adds `.open` and `.release` callbacks (module reference counting via `try_module_get(THIS_MODULE)` / `module_put()` or close) to prevent module unloading while in use.  
- On init: uses `proc_set_size()` and `proc_set_user()` to set file size and ownership.  
**Key learning:** more advanced procfs uses, safe module unload protection, kernel API version awareness, buffer limits.  

---

## Build & Usage  
### Build  
```bash
cd kernel_pseudo_filesystem
make
sudo insmod helloworld_procfs.ko
sudo dmesg -t | tail ‑10
ls /proc | grep helloworld
cat /proc/helloworld


