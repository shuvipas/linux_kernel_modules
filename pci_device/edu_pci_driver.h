#ifndef EDU_DRIVER
#define EDU_DRIVER
#include <linux/ioctl.h>
#include <linux/types.h> 

#define EDU_IOCTL_BASE 'e'

#define EDU_IOCTL_ID              _IOR(EDU_IOCTL_BASE, 1, __u32)
#define EDU_IOCTL_LIVE_CHECK      _IOWR(EDU_IOCTL_BASE, 2, __u32)
#define EDU_IOCTL_FACTORIAL       _IOWR(EDU_IOCTL_BASE, 3, __u32)
#define EDU_IOCTL_RAISE_IRQ       _IOC(_IOC_WRITE, EDU_IOCTL_BASE, 4, 0)
#define EDU_IOCTL_DMA_TO_DEVICE   _IOW(EDU_IOCTL_BASE, 5, __u32[3]) /* len, src, dst */
#define EDU_IOCTL_DMA_FROM_DEVICE _IOR(EDU_IOCTL_BASE, 6, __u32[3])


#endif