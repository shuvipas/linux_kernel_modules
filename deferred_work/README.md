# Deferred Work Modules

This folder contains Linux kernel modules demonstrating deferred work mechanisms, timers, workqueues, and process monitoring using a character device interface. It includes two kernel modules and a user-space program to interact with the `deferred` module.

## Folder Structure

* `deferred` - Character device module supporting deferred work, memory allocation, and process monitoring.  
* `timer` - Simple periodic kernel timer module.  
* `user` - User-space program to interact with the `deferred` module.

## Modules Overview

### deferred

A character device driver that provides deferred execution and process monitoring functionalities through ioctl commands.

**Functionalities:**

* Set a timer to execute after a specified delay (`MY_IOCTL_TIMER_SET`)  
* Cancel a running timer (`MY_IOCTL_TIMER_CANCEL`)  
* Allocate memory and perform deferred work after a delay (`MY_IOCTL_TIMER_ALLOC`)  
* Monitor a specific process by PID and report when it terminates (`MY_IOCTL_TIMER_MON`)  

**Key Concepts:**

* Character device registration and management  
* IOCTL interface implementation  
* Timers with `struct timer_list`  
* Deferred work using `struct work_struct`  
* Spinlocks and linked list management for concurrent access  
* Kernel memory allocation with `kmalloc` and `kfree`  
* Accessing and managing `task_struct`  

### timer

A simple kernel module demonstrating a periodic timer.

**Functionalities:**

* Sets up a timer that triggers every second.  
* Increments a counter and prints a message to the kernel log each time the timer fires.  
* Reschedules itself automatically for periodic execution.  

**Key Concepts:**

* Kernel timers using `struct timer_list`  
* Scheduling periodic execution with `mod_timer`  
* Using `jiffies` for timing  
* Kernel logging with `pr_info`  
* Proper cleanup with `timer_delete_sync`  

### User-space Program

An interactive command-line program to test the `deferred` module.  
It communicates with `/dev/deferred` and allows the user to:

* Set a timer  
* Cancel a timer  
* Allocate memory with deferred execution  
* Monitor a specific PID  
* Quit the program  

**Supported Commands:**

* `s <seconds>` - set timer to run after the specified number of seconds  
* `c` - cancel the timer  
* `a <seconds>` - allocate memory after the specified number of seconds  
* `p <pid>` - monitor a specific process by PID  
* `q` - quit the program  

## Build and Usage

### Kernel Modules

```bash
# Build all modules
make

# Insert deferred module
sudo insmod deferred/deferred.ko

# Insert timer module
sudo insmod timer/timer.ko

# Check kernel logs
dmesg | tail -n 30

# Remove modules
sudo rmmod deferred
sudo rmmod timer

```

# User-space Program
```bash
# Compile the program
gcc -o deferred_user user/deferred_user.c

# Run the program
./deferred_user
