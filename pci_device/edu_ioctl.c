#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "edu_pci_driver.h"

#define DEVICE_PATH "/dev/edu"

void test_device_id(int fd) {
    uint32_t id;
    if (ioctl(fd, EDU_IOCTL_ID, &id) < 0) {
        perror("EDU_IOCTL_ID failed");
        return;
    }
    printf("Device ID: 0x%08x\n", id);
}

void test_live_check(int fd) {
    uint32_t test_val = 0x12345678;
    uint32_t result = test_val;
    
    if (ioctl(fd, EDU_IOCTL_LIVE_CHECK, &result) < 0) {
        perror("EDU_IOCTL_LIVE_CHECK failed");
        return;
    }
    printf("Live check: 0x%08x -> 0x%08x (expected: 0x%08x)\n", 
           test_val, result, ~test_val);
}

void test_factorial(int fd, uint32_t n) {
    uint32_t result = n;
    
    printf("Computing factorial of %u...\n", n);
    if (ioctl(fd, EDU_IOCTL_FACTORIAL, &result) < 0) {
        perror("EDU_IOCTL_FACTORIAL failed");
        return;
    }
    printf("Factorial result: %u! = %u\n", n, result);
}

void test_raise_irq(int fd, uint32_t irq_type) {
    printf("Raising IRQ 0x%x...\n", irq_type);
    if (ioctl(fd, EDU_IOCTL_RAISE_IRQ, irq_type) < 0) {
        perror("EDU_IOCTL_RAISE_IRQ failed");
    }
}

void test_dma_transfer(int fd, int to_device, uint32_t len) {
    uint32_t params[3];
    uint8_t *buffer;
    int ret;
    
    // Allocate test buffer
    buffer = malloc(len);
    if (!buffer) {
        perror("malloc failed");
        return;
    }
    
    // Fill with pattern if writing to device
    if (to_device) {
        for (uint32_t i = 0; i < len; i++) {
            buffer[i] = i % 256;
        }
    }
    
    params[0] = len;                  // Length
    params[1] = (uint32_t)(uintptr_t)buffer; // Source (host)
    params[2] = 0x40000;              // Destination (device buffer)
    
    printf("Testing %s transfer (%u bytes)...\n", 
           to_device ? "host-to-device" : "device-to-host", len);
    
    if (to_device) {
        ret = ioctl(fd, EDU_IOCTL_DMA_TO_DEVICE, params);
    } else {
        ret = ioctl(fd, EDU_IOCTL_DMA_FROM_DEVICE, params);
    }
    
    if (ret < 0) {
        perror("DMA operation failed");
        free(buffer);
        return;
    }
    
    if (!to_device) {
        // Verify data if reading from device
        printf("First 16 bytes of received data:\n");
        for (int i = 0; i < 16 && i < len; i++) {
            printf("%02x ", buffer[i]);
        }
        printf("\n");
    }
    
    free(buffer);
}

int main() {
    int fd;
    
    // Open device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }
    
    printf("=== EDU Device Test ===\n");
    
    // Test basic functionality
    test_device_id(fd);
    test_live_check(fd);
    
    // Test factorial computation
    test_factorial(fd, 5);
    test_factorial(fd, 10);
    
    // Test IRQ generation
    test_raise_irq(fd, 0x1);
    test_raise_irq(fd, 0x2);
    
    // Test DMA operations
    test_dma_transfer(fd, 1, 100);  // Host -> Device
    test_dma_transfer(fd, 0, 100);  // Device -> Host
    
    // Test edge cases
    test_dma_transfer(fd, 1, 4096); // Max buffer size
    test_factorial(fd, 0);          // 0! case
    
    close(fd);
    return EXIT_SUCCESS;
}