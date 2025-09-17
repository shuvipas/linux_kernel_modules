# Keylogger Module

A simple Linux kernel module that implements a character device `/dev/kbd` for logging keystrokes using the i8042 keyboard controller. 

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
