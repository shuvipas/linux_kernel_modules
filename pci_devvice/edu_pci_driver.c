#include <linux/kernel.h>
#include<linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>

//todo clean up the probe function (irq and all fails)
// remove fuction
// 

#define DEV_NAME "qemu_edu"
#define VENDOR_ID 0x1234
#define DEVICE_ID 0X11e8
#define EDU_MASK 28
#define EDU_DMA_BUF_SIZE 4096

#define EDU_ADDR_INTR_STAT 0X24
#define EDU_ADDR_INTR_RAISE 0X60
#define EDU_ADDR_INTR_ACK 0X64
struct edu_device{
    dma_addr_t dma_handle;
    void *dma_virt_addr;
    void __iomem * p_iomem;
    unsigned int irq_line;
};
/*PCI Base Address Register*/
static int edu_bar = 0; 
static struct edu_device *edu_dev;
static struct pci_device_id edu_id[] = {
    {PCI_DEVICE(VENDOR_ID,DEVICE_ID),},
    {0,}
};
MODULE_DEVICE_TABLE(pci, edu_id); /*exports pci_device_id table to userspace for automatically load your driver*/

static irq_handler_t (int irq, void *dev_id){

}
static void edu_remove(struct pci_dev *pdev){
    pr_info("edu device remove func\n");
    pci_clear_master(pdev);
    pci_disable_device(pdev);
    pci_release_region(pdev,edu_bar);
}
/* gets called to handle the device */
static int edu_probe(struct pci_dev *pdev, const struct pci_device_id *id){
    int itq_alloc_vec;
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
    if (edu_dev->p_iomem == IOMEM_ERR_PTR) {
        err = PTR_ERR(edu_dev->iomem);
        goto disable_dev;
    }

    if(dma_set_mask_and_coherent(pdev, DMA_BIT_MASK(EDU_MASK))){
        goto disable_dev;
        return -1;
    }
    edu_dev->dma_virt_addr = dmam_alloc_coherent(
        &pdev->dev, EDU_DMA_BUF_SIZE, &edu_dev->dma_bus_addr, GFP_KERNEL);
    if (!edu_dev->dma_virt_addr) {
        err = -ENOMEM;
        goto release_regions;
    }
    // itq_alloc_vec = pci_alloc_irq_vectors(pdev, unsigned int min_vecs, unsigned int max_vecs, PCI_IRQ_INTX);
    /*min_vecs == max_vec becuse the device uses only one interrupt source*/
    itq_alloc_vec = pci_alloc_irq_vectors(pdev, 1,1, PCI_IRQ_MSI | PCI_IRQ_INTX);
    if (itq_alloc_vec < 1){
        goto irq_alloc;
    }


    /* Clear pending IRQ state in hardware before enabling handler */
    // iowrite32(~0u, edu_dev->iomem + EDU_REG_INT_ACK);
    // writel(arg, dev->iomem + EDU_ADDR_IRQ_RAISE);


    edu_dev->irq_line = pci_irq_vector(pdev, 0);/*i have only 1 vec so 0 for msi and intx*/
    if(edu_dev->irq_line==EINVAL){

    }


    if(request_irq(edu_dev->irq_line, edu_irq_handler, IRQF_SHARED, DEV_NAME,edu_dev)){
        goto irq_fin;
    }
    // 11. Enable interrupts at the device level (hardware)
    // Now that the interrupt handler is successfully registered with the kernel,
    // re-enable the device to start generating interrupts.
    // dev_info(&pdev->dev, "Enabling interrupts at device level...\n");
    // iowrite32(EDU_ALL_INT_MASK, edu_dev->regs_base + EDU_REG_INT_ENABLE);


    return 0;


    // release_regions:
    //     pci_release_regions(pdev);
    irq_alloc:
        pci_free_irq_vectors(pdev); /*pci_alloc_irq_vectors*/
    irq_fin:
        free_irq(edu_dev->irq_line, edu_dev) /*request_irq*/
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

static int __init pci_dev_init(void){
    
    edu_dev = kzalloc(sizeof(*edu_dev), GFP_KERNEL);
    if (!edu_dev) 
        err = -ENOMEM;

    if(pci_register_driver(&edu_pci_driver)){
        kfree(edu_dev);
        return -1;
    }
    return 0;
}

static void __exit pci_dev_exit(void){
    pci_unregister_driver(&edu_pci_driver);
}

module_init(pci_dev_init);
module_exit(pci_dev_exit);
MODULE_DESCRIPTION("QEMU EDU (PCI) device driver");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");