# Keylogger Module

This folder contains a Linux kernel module that implements a simple keyboard logger using a character device interface. The module captures key presses from the PS/2 keyboard and stores them in a buffer accessible from user space.

## Module Overview

### keylogger

The `keylogger` module registers a character device `/dev/keylogger` and intercepts key presses via the PS/2 keyboard IRQ (IRQ 1). It converts scancodes to ASCII characters for alphanumeric keys and basic punctuation and stores them in a circular buffer.

**Functionalities:**

* Captures keyboard input and stores it in a buffer  
* Provides read access to retrieve captured keystrokes  
* Supports a `clear` command via write to reset the buffer  
* Logs module load, unload, and interrupt events to the kernel log  

**Key Kernel Concepts Demonstrated:**

* Interrupt handling using `request_irq` and custom IRQ handler  
* Low-level keyboard I/O via PS/2 controller ports (`inb`)  
* Spinlocks for safe concurrent access to the buffer  
* Circular buffer management  
* Character device registration and `file_operations`  
* Kernel to user-space communication using `copy_to_user` and `copy_from_user`  


## Build & Install

1. Compile the module  
    ```bash
   make
2. Insert the module
     ```bash
    sudo insmod kbd.ko
3. Create the device node
    
    sudo mknod /dev/kbd c 42 0
    sudo chmod 666 /dev/kbd   # allows non root access
## Usage
Read captured keystrokes

    cat /dev/kbd
    
Clear the log buffer

    echo "clear" > /dev/kbd

## Implementation Details

Registers a character device with major 42 and minor 0
Installs an interrupt handler for the i8042 keyboard IRQ 1
Converts scancodes into ASCII for alphanumeric keys space and enter
Maintains a circular buffer BUFFER_SIZE 1024
Provides a read method to fetch keystrokes and a write method to handle the "clear" command
