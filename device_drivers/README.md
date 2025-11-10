# Character Device Drivers

This folder contains Linux kernel modules demonstrating different character device implementations, including basic read/write, IOCTL handling, and synchronization with wait queues.

Included modules:

* `char_dev` – A simple character device with read, write, and ioctl operations.  
* `lkt_char_dev` – A SO2‑style character device illustrating advanced features like atomic open, blocking IOCTL, and wait queues.

---

## Overview of the Modules

### `chardev.c` – simple read‑only char device  
- Creates a character device named `chardev` (via `#define DEVICE_NAME "chardev"`).  
- Uses `register_chrdev(0, …)` to get a dynamic major number.  
- Implements `open`, `read`, `write`, `release`.  
  - `open`: increments a counter and writes: “I already told you X times Hello world!” into a buffer.  
  - `read`: copies the message to user space via `put_user()`.  
  - `write`: not supported — returns `‑EINVAL`.  
  - `release`: resets the atomic flag for exclusive access.  
- Uses `atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED)` to enforce single‑open.  
- Creates device node: uses `class_create()`, `device_create()`.  
**Key learning:** simple char device infrastructure, major/minor, basic read from device.

### `char_dev` module – I/O char device + IOCTL  
- Device named `char_dev`.  
- Prevents concurrent access (via `atomic_t already_open`).  
- Implements: `open` (with `try_module_get()`), `release` (with `module_put()`), `read`, `write`, `unlocked_ioctl`.  
- Supports IOCTL commands defined in `char_device.h`:  
  - `IOCTL_SET_MSG` – set a message (via user‑space buffer)  
  - `IOCTL_GET_MSG` – get message  
  - `IOCTL_GET_NTH_BYTE` – get the Nth byte of the message  
- Demonstrates how to bridge user space ↔ kernel space with IOCTLs.  
- Build & usage: `sudo insmod char_device.ko`, ensure `/dev/char_dev` exists or create it (`chmod 666 /dev/char_dev` if necessary).  
- Test with sample user‑space program (see section 5).  
**Key learning:** IOCTL interface, more realistic device operations, proper open/release.

### `ioctltest` module – `alloc_chrdev_region()` + `cdev` + multiple IOCTLs  
- Uses `alloc_chrdev_region()` + `cdev_init()` + `cdev_add()` (the “modern” API) to allocate device numbers.  
- Manages a per‑open private data structure via `kmalloc()`, uses `rwlock_t` to protect the value field.  
- Supports IOCTL commands (via `copy_from_user()`, `copy_to_user()`):  
  - `IOCTL_VALSET`, `IOCTL_VALGET`, `IOCTL_VALGET_NUM`, `IOCTL_VALSET_NUM`.  
- Implements `read` to return the current value repeated for requested count.  
- Enforces major/minor via `alloc_chrdev_region()` rather than static major.  
**Key learning:** advanced character device registration, use of locks in kernel space, full IOCTL support and good separation of concerns.

## User‑Space Test Application (for the IOCTL module)  
For the `char_dev` or `ioctltest` modules you’ll find a user‑space program (e.g., `userspace_ioctl.c`) which:  
- Opens `/dev/char_dev` (or `/dev/<your_device>`).  
- Calls `ioctl_set_msg()` to send a message to the driver.  
- Calls `ioctl_get_nth_byte()` to retrieve each byte until the null terminator.  
- Calls `ioctl_get_msg()` to retrieve the entire message.

**Key Concepts Demonstrated:**  
* Character device registration using `register_chrdev`  
* Exclusive device access via `atomic_t`  
* Kernel ↔ user‑space communication (`copy_to_user`, `copy_from_user`)  
* IOCTL handling (`unlocked_ioctl`)

### Build & Install  
```bash
make
sudo insmod char_device.ko
sudo chmod 666 /dev/char_dev

gcc userspace_ioctl.c ‑o userspace_ioctl
./userspace_ioctl

sudo rmmod <module_name>
dmesg | tail ‑n20
