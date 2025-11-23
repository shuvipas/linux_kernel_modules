# Kernel API Modules

This folder contains example Linux kernel modules that demonstrate various kernel APIs and mechanisms such as module parameters, process iteration, memory allocation, and kernel threading.

## Modules Overview

### cmd_mod
Demonstrates the use of module parameters (module_param, module_param_array) with different permissions and data types.  
Prints parameter values on module load.


### kthread
Implements a simple kernel thread using kthread_run and wait_event_interruptible.  
The thread waits on a condition and exits gracefully when signaled.

### sched_spin
Demonstrates a spinlock misuse scenario, attempting to sleep while holding a spinlock.  
This serves as an educational example of atomic context behavior.


### memory
Allocates and tracks small structures using kmalloc and kfree to store task information such as PID and timestamps.



### proc_info
Prints information about the current process and all processes in the system, including virtual memory area (VMA) details.  
Adapts to different kernel versions for VMA iteration.

