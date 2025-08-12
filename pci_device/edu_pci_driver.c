#include <linux/kernel.h>
#include<linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/fs.h> 

//todo clean up the probe function (irq and all fails)
// remove fuction
// edu_irq_handler funct

#define DEV_NAME "qemu_edu"
#define VENDOR_ID 0x1234
#define DEVICE_ID 0X11e8
#define EDU_MASK 28
#define EDU_DMA_BUF_SIZE 4096
#define EDU_ADDR_ID 0x0

#define EDU_ADDR_INTR_STAT 0X24
#define EDU_ADDR_INTR_RAISE 0X60
#define EDU_ADDR_INTR_ACK 0X64

#define EDU_ADDR_LIVE_CHECK 0x4
#define EDU_ADDR_FACTORIAL 0x8
#define EDU_ADDR_STAT_REG 0x20
#define EDU_COMPUTE_FACTORIAL 0x1
#define EDU_RAISE_IRQ_FACTORIAL 0x80

#define EDU_ADDR_DMA_SRC 0x80
#define EDU_ADDR_DMA_DIST 0x88
#define EDU_ADDR_DMA_SIZE 0x90
#define EDU_ADDR_DMA_CMD 0x98
#define EDU_DMA_IRQ_FLAG 0x100

#define EDU_DMA_CMD_TRANSFER 0x1
#define EDU_DMA_CMD_DIRC 0x2
#define EDU_DMA_CMD_RAISE_IRQ 0x4


struct edu_device{
    dma_addr_t dma_handle;
    dma_addr_t dma_bus_addr;
    void *dma_virt_addr;
    void __iomem * p_iomem;
    unsigned int irq_line;

    wait_queue_head_t factorial_wait;
    wait_queue_head_t dma_wait;
    atomic_t factorial_done;
    atomic_t dma_done;

    u32 factorial_result;
    
    dev_t dev_num;
    struct cdev edu_cdev;
    struct class *cls;
    struct device *dev;

};
/*PCI Base Address Register*/
static int edu_bar = 0; 
static struct edu_device *edu_dev;
static struct pci_device_id edu_id[] = {
    {PCI_DEVICE(VENDOR_ID,DEVICE_ID),},
    {0,}
};
MODULE_DEVICE_TABLE(pci, edu_id); /*exports pci_device_id table to userspace for automatically load your driver*/

    /* ======================================================================================================
                                                pci structure
       ======================================================================================================
    */


static irqreturn_t edu_irq_handler(int irq, void *dev_id){
    //todo atomic and wake_up_interruptible did i do correctly
    irqreturn_t ret_val = IRQ_NONE;
    struct edu_device *pdev = dev_id;
    u32 irq_status;
    u32 dma_cmd;
    u32 factorial_val;
    if(!pdev||!pdev->p_iomem){
        return ret_val;
    }
    irq_status = readl(pdev->p_iomem+ EDU_ADDR_INTR_STAT);
    
    // pr_info("EDU IRQ: status=0x%x\n", irq_status);

    if(irq_status & EDU_RAISE_IRQ_FACTORIAL){
        // handle_factorial_computation(pdev);
        factorial_val = readl(pdev->p_iomem+EDU_ADDR_FACTORIAL);
        pr_info("qemu edu: factorial computation: %x\n",factorial_val);
        // atomic_set(&pdev->dma_done, 1);
        wake_up_interruptible(&pdev->factorial_wait);
        ret_val = IRQ_HANDLED;
    }
    if (irq_status & EDU_DMA_IRQ_FLAG) {
        dma_cmd = readl(pdev->p_iomem + EDU_ADDR_DMA_CMD);
        pr_info("DMA completed, final cmd reg: 0x%x\n", dma_cmd);
        ret_val = IRQ_HANDLED;
        wake_up_interruptible(&pdev->dma_wait);
     }
    writel(irq_status, pdev->p_iomem + EDU_ADDR_INTR_ACK);
    return ret_val;
}

static void edu_remove(struct pci_dev *pdev){
    pr_info("edu device remove func\n");
    free_irq(edu_dev->irq_line, edu_dev); /*request_irq*/
    pci_free_irq_vectors(pdev); /*pci_alloc_irq_vectors*/
    pci_release_regions(pdev);
    pci_disable_device(pdev);
}
/* gets called to handle the device */
static int edu_probe(struct pci_dev *pdev, const struct pci_device_id *id){
    int irq_alloc_vec;
    pr_info("edu device probe func\n");
    // int err =0;

    // err = pci_enable_device(pdev);
    if(pci_enable_device(pdev)){
        return -1;
    }

    pci_set_master(pdev);
    /*Mark all PCI regions associated with PCI
     device pdev as being reserved by owner name.*/
    // err = pci_request_regions(pdev,edu_bar, DEV_NAME);
    
    edu_dev->p_iomem = pcim_iomap_region(pdev, edu_bar, DEV_NAME);
    if (IS_ERR(edu_dev->p_iomem)) {
        goto disable_dev;
    }

    if(dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(EDU_MASK))){
        goto disable_dev;
        return -1;
    }
    edu_dev->dma_virt_addr = dmam_alloc_coherent(
        &pdev->dev, EDU_DMA_BUF_SIZE, &edu_dev->dma_bus_addr, GFP_KERNEL);
    if (!edu_dev->dma_virt_addr) {
        goto release_regions;
    }
    // irq_alloc_vec = pci_alloc_irq_vectors(pdev, unsigned int min_vecs, unsigned int max_vecs, PCI_IRQ_INTX);
    /*min_vecs == max_vec becuse the device uses only one interrupt source*/
    irq_alloc_vec = pci_alloc_irq_vectors(pdev, 1,1, PCI_IRQ_MSI | PCI_IRQ_INTX);
    if (irq_alloc_vec < 1){
        goto irq_free;
    }


    /* Clear pending IRQ state in hardware before enabling handler */
    writel(~0u, edu_dev->p_iomem + EDU_ADDR_INTR_RAISE);


    edu_dev->irq_line = pci_irq_vector(pdev, 0);/*i have only 1 vec so 0 for msi and intx*/
    if(edu_dev->irq_line==EINVAL){
        goto irq_free;
    }


    if(request_irq(edu_dev->irq_line, edu_irq_handler, IRQF_SHARED, DEV_NAME,edu_dev)){
        goto irq_free;
    }

    return 0;


    
    irq_free:
        free_irq(edu_dev->irq_line, edu_dev); /*request_irq*/
        pci_free_irq_vectors(pdev); /*pci_alloc_irq_vectors*/
    release_regions:
        pci_release_regions(pdev);
    disable_dev:
        pci_disable_device(pdev);
    return -1;

}

static struct pci_driver edu_pci_driver = {
    .name = DEV_NAME,
    .id_table = edu_id,
    .probe = edu_probe,
    .remove = edu_remove,
    };

    /* ======================================================================================================
                                                char ops
       ======================================================================================================
    */
   

// static ssize_t dma_write(struct file *file, const char __user *buffer,
//                             size_t length, loff_t *offset)
static int edu_open(struct inode *inode, struct file *file){
    u32 val = 17; /*random val to check livenss*/
    u32 not_val = ~val;
    writel(val, edu_dev->p_iomem + EDU_ADDR_LIVE_CHECK);
    not_val = readl(edu_dev->p_iomem+ EDU_ADDR_LIVE_CHECK);
    if(~val != not_val){
        pr_info("ERROR: canot open qemu edu device\n");
        return 1;
    }
    pr_info("edu device opened\n");
    return 0;
}
static int edu_release(struct inode *inode, struct file *file)
{
    pr_info("edu device closed\n");
    return 0;
}
static struct file_operations char_ops = {
    .owner = THIS_MODULE,
    // .read = device_read,
    // .write = dma_write,
    // .unlocked_ioctl = device_ioctl,
    .open = edu_open,
    .release = edu_release, 
    // .mmap = insted of read/write becuse mmio
};

static int __init pci_dev_init(void){
    int ret_val;
    edu_dev = kzalloc(sizeof(*edu_dev), GFP_KERNEL);
    if (!edu_dev){
        pr_info("ERROR: memory allocation failed\n");
        return -ENOMEM;
    }
    init_waitqueue_head(&edu_dev->factorial_wait);
    init_waitqueue_head(&edu_dev->dma_wait);
    atomic_set(&edu_dev->factorial_done, 0);
    atomic_set(&edu_dev->dma_done, 0);

    /*Allocate device numbers*/
    ret_val = alloc_chrdev_region(&edu_dev->dev_num, 0, 1, DEV_NAME);
    if (ret_val <= 0){
        goto free_dev;
    }
    cdev_init(&edu_dev->edu_cdev, &char_ops);
    edu_dev->edu_cdev.owner = THIS_MODULE;
    ret_val = cdev_add(&edu_dev->edu_cdev, edu_dev->dev_num, 1);
    if (ret_val < 0){
        goto unregister_chrdev;
    }
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    edu_dev->cls = class_create(DEV_NAME);
    #else
    edu_dev->cls = class_create(THIS_MODULE, DEV_NAME);
    #endif
    if (IS_ERR(edu_dev->cls)) {
        ret_val = PTR_ERR(edu_dev->cls);
        pr_info("ERROR: Cannot create the struct class\n");
        goto del_cdev;
    }

    edu_dev->dev = device_create(edu_dev->cls, NULL, edu_dev->dev_num, edu_dev, DEV_NAME);
    if (IS_ERR(edu_dev->dev)) {
        ret_val = PTR_ERR(edu_dev->dev);
        pr_info("ERROR: device_create failed: %d\n", ret_val);
        goto destroy_class;
    }

    ret_val = pci_register_driver(&edu_pci_driver);
    if(ret_val){
        goto destroy_device;
    }
    

    return 0;

    destroy_device:
        device_destroy(edu_dev->cls, edu_dev->dev_num);
    destroy_class:
        class_destroy(edu_dev->cls);
    del_cdev:
        cdev_del(&edu_dev->edu_cdev);
    unregister_chrdev:
        unregister_chrdev_region(edu_dev->dev_num, 1);
    free_dev:
        kfree(edu_dev);
        return ret_val;
}

static void __exit pci_dev_exit(void){  
    device_destroy(edu_dev->cls, edu_dev->dev_num);
    class_destroy(edu_dev->cls);
    cdev_del(&edu_dev->edu_cdev);
    unregister_chrdev_region(edu_dev->dev_num, 1);
    pci_unregister_driver(&edu_pci_driver);
    kfree(edu_dev);
}

module_init(pci_dev_init);
module_exit(pci_dev_exit);
MODULE_DESCRIPTION("QEMU EDU (PCI) device driver");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");