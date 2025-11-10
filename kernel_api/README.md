# Kernel API Modules

This folder contains example Linux kernel modules that demonstrate various kernel APIs and mechanisms such as module parameters, process iteration, memory allocation, and kernel threading.

Each module is self-contained and illustrates a specific aspect of Linux kernel programming.

## Modules Overview

### cmd_mod
Demonstrates the use of module parameters (module_param, module_param_array) with different permissions and data types.  
Prints parameter values on module load.

Concepts:
* Module parameters and permissions  
* Kernel logging with pr_info  
* Parameter descriptions visible through modinfo  

### kthread
Implements a simple kernel thread using kthread_run and wait_event_interruptible.  
The thread waits on a condition and exits gracefully when signaled.

Concepts:
* Kernel threads (kthread_run, kthread_stop)  
* Wait queues  
* Atomic variables  

### sched_spin
Demonstrates a spinlock misuse scenario, attempting to sleep while holding a spinlock.  
This serves as an educational example of atomic context behavior.

Concepts:
* Spinlocks (DEFINE_SPINLOCK, spin_lock, spin_unlock)  
* Sleeping vs. atomic context  

### memory
Allocates and tracks small structures using kmalloc and kfree to store task information such as PID and timestamps.

Concepts:
* Dynamic memory allocation in kernel space  
* Accessing current, parent, and next_task  

### proc_info
Prints information about the current process and all processes in the system, including virtual memory area (VMA) details.  
Adapts to different kernel versions for VMA iteration.

Concepts:
* Iterating through processes using for_each_process  
* Working with task_struct and vm_area_struct  
* Kernel version dependent macros  


## Usage

```bash
# Build
make

# Insert a module
sudo insmod module_name.ko

# View kernel logs
dmesg | tail -n 30

# Remove a module
sudo rmmod module_name
