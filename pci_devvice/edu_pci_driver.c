#include <linux/kernel.h>
#include<linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>

//todo clean up the probe function (irq and all fails)
// remove fuction
// edu_irq_handler funct

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
    wait_queue_head_t irq_wait_queue;
};
/*PCI Base Address Register*/
static int edu_bar = 0; 
static struct edu_device *edu_dev;
static struct pci_device_id edu_id[] = {
    {PCI_DEVICE(VENDOR_ID,DEVICE_ID),},
    {0,}
};
MODULE_DEVICE_TABLE(pci, edu_id); /*exports pci_device_id table to userspace for automatically load your driver*/

static irq_handler_t edu_irq_handler(int irq, void *dev_id){
    irq_handler_t ret;
    struct edu_device *pdev = dev_id;
    u32 irq_val = readl(pdv->p_iomem+ EDU_ADDR_INTR_STAT);

    //todo case and do the interrupt with the char ops
    // ret = IRQ_HANDLED;
    // default: ret = IRQ_NONE ;

    
    writel(irq_val, dev->iomem + EDU_ADDR_INTR_ACK);
    
    return ret;
}

static void edu_remove(struct pci_dev *pdev){
    pr_info("edu device remove func\n");
    free_irq(edu_dev->irq_line, edu_dev) /*request_irq*/
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
    // irq_alloc_vec = pci_alloc_irq_vectors(pdev, unsigned int min_vecs, unsigned int max_vecs, PCI_IRQ_INTX);
    /*min_vecs == max_vec becuse the device uses only one interrupt source*/
    irq_alloc_vec = pci_alloc_irq_vectors(pdev, 1,1, PCI_IRQ_MSI | PCI_IRQ_INTX);
    if (irq_alloc_vec < 1){
        goto irq_alloc;
    }


    /* Clear pending IRQ state in hardware before enabling handler */
    writel(~0u, dev->iomem + EDU_ADDR_INTR_RAISE);


    edu_dev->irq_line = pci_irq_vector(pdev, 0);/*i have only 1 vec so 0 for msi and intx*/
    if(edu_dev->irq_line==EINVAL){
        goto irq_free;
    }


    if(request_irq(edu_dev->irq_line, edu_irq_handler, IRQF_SHARED, DEV_NAME,edu_dev)){
        goto irq_free;
    }

    return 0;


    
    irq_free:
        free_irq(edu_dev->irq_line, edu_dev) /*request_irq*/
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