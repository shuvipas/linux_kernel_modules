#include <linux/kernel.h>
#include<linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>

#define DEV_NAME "qemu_edu"
#define VENDOR_ID 0x1234
#define DEVICE_ID 0X11e8


static struct pci_device_id edu_id[] = {
    {PCI_DEVICE(VENDOR_ID,DEVICE_ID),},
    {0,}
};
MODULE_DEVICE_TABLE(pci, edu_id); /*exports pci_device_id table to userspace for automatically load your driver*/


static void edu_remove(struct pci_dev *dev){
    pr_info("edu device remove func\n");
    pci_disable_device(dev);
    
}
/* gets called to handle the device */
static int edu_probe(struct pci_dev *dev, const struct pci_device_id *id){
    pr_info("edu device probe func\n");
    int err =0;

    err = pci_enable_device(dev);
    if(err){
        edu_remove(dev);
        return err;
    }
    return 0;

}



static struct pci_driver edu_pci_driver = {
    .name = DEV_NAME,
    .id_table = edu_id,
    .probe = edu_probe,
    .remove = edu_remove,
    };

static int __init pci_dev_init(void){
    return pci_register_driver(&edu_pci_driver);
}

static void __exit pci_dev_exit(void){
    pci_unregister_driver(&edu_pci_driver);
}

module_init(pci_dev_init);
module_exit(pci_dev_exit);
MODULE_DESCRIPTION("QEMU EDU (PCI) device driver");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");