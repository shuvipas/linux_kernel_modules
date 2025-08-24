[How To Write Linux PCI Drivers linux docs](https://docs.kernel.org/PCI/pci.html)



# Driver for a PCI device 
this is qemu edu is a virtual PCI device, specs [here](https://www.qemu.org/docs/master/specs/edu.html) implemented [here](https://github.com/qemu/qemu/blob/v2.7.0/hw/misc/edu.c),  behaves very similar to the PCI bridge present in the COMBO6 cards developed under the Liberouter wings. The driver demonstrates interaction with a virtual PCI device through MMIO, IRQs, and DMA operations.

## Overview

The QEMU EDU device is a simple virtual PCI device including:

- PCI device initialization and memory mapping
- Register access via MMIO
- Interrupt handling
- DMA configuration and transfer

## Device Summary

- **PCI Vendor ID:** `0x1234`
- **PCI Device ID:** `0x11e8`
- **MMIO Region:** 1 MB (via BAR0)
- **Internal buffer for DMA:** 4 KB at offset `0x40000`

## Functional Features

### 1. Device Identification and Liveness Check

- `0x00` (RO): Identification register  
  Returns a value in the format `0xRRrr00edu`, where `RR` is major version and `rr` is minor version.
  
- `0x04` (RW): Liveness check  
  Write a value and read back its bitwise inverse to confirm device communication.

### 2. Factorial Computation Unit

- `0x08` (RW): Write a number here. After the corresponding bit in the status register is cleared, the factorial is computed and stored at the same location.
  
- `0x20` (RW): Status register  
  - Bit `0x01`: Indicates computation in progress (read-only)  
  - Bit `0x80`: Request an interrupt upon completion

### 3. Interrupt Controller

- `0x60` (WO): Raise interrupt by writing a value.
- `0x24` (RO): Read interrupt status.
- `0x64` (WO): Acknowledge and clear interrupt by writing the same value.

Interrupts must be acknowledged in the ISR (Interrupt Service Routine) to avoid repeated triggering. Both INTx and MSI are supported.

### 4. DMA Engine

Registers:
- `0x80` (RW): Source address
- `0x88` (RW): Destination address
- `0x90` (RW): Transfer size
- `0x98` (RW): DMA command register  
  - Bit `0x01`: Start transfer  
  - Bit `0x02`: Direction (0: RAM → EDU, 1: EDU → RAM)  
  - Bit `0x04`: Request interrupt on completion

DMA operation is performed between RAM and the device’s internal 4 KB buffer.

