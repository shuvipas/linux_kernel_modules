#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

struct ioctl_arg{
    unsigned int val;
};
/* 8 bit num to uniquely identify driver's ioctl commands*/
#define IOC_MAGIC '\x66'
/*io set/get write/read*/
/*Pass a full structure from kernel to user*/
#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg)
/*pass a single num*/
#define IOCTL_VALGET_NUM _IOR(IOC_MAGIC, 2, int)
#define IOCTL_VALSET_NUM _IOW(IOC_MAGIC, 3, int)

#define IOCTL_VAL_MAXNR 3 /*valid ioctl commands range*/
#define DRIVER_NAME "ioctltest"

static unsigned int test_ioctl_major =0;
static unsigned int num_of_dev =1;
static struct cdev test_ioctl_cdev;
static int ioctl_num =0;

struct test_ioctl_data{
    unsigned char val;
    rwlock_t lock;
};

static long test_ioctl_ioctl(struct file* fp, unsigned int cmd, unsigned long arg){
    struct test_ioctl_data *ioctl_data = fp->private_data;
    int retval =0;
    unsigned char val;
    struct ioctl_arg data;
    memset(&data,0,sizeof(data));
    switch (cmd)
    {
    case IOCTL_VALSET:
        if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
            retval = -EFAULT;
            goto done;
        }
        pr_alert("IOCTL set val:%x .\n", data.val);
        write_lock(&ioctl_data->lock);
        ioctl_data->val = data.val;
        write_unlock(&ioctl_data->lock);
        break;
    case IOCTL_VALGET:
        read_lock(&ioctl_data->lock);
        val =ioctl_data->val;
        read_unlock(&ioctl_data->lock);
        data.val = val;

        if (copy_to_user((int __user *)arg, &data, sizeof(data))) {
            retval = -EFAULT;
            goto done;
        }
        break;
    case IOCTL_VALGET_NUM:
        retval = __put_user(ioctl_num, (int __user *)arg);
        break;
    case IOCTL_VALSET_NUM:
        ioctl_num = arg;
        break;
    default:
        retval = -ENOTTY;
    }
    done:
    return retval;
}

static int test-ioctl_close(struct inode *inode, struct file* fp){
    pr_alert("%s call.\n", __func__);/*__func__= name of the current function*/
    if(fp->privatr_data){
        kfree(fp->privatr_data);
        fp->privatr_data =NULL;
    }
    return 0;
}

static int test-ioctl_open(struct inode *inode, struct file* fp){
    struct test_ioctl_data *ioctl_data;
    pr_alert("%s call.\n", __func__);
    ioctl_data = kmalloc(sizeof(struct test_ioctl_data),GFP_KERNEL);/*GFP_KERNEL- Get Free Pages for Kernel*/
    if(!ioctl_data)
        return -ENOMEM;
    rwlock_init(&ioctl_data->lock);
    ioctl_data->val = 0xFF;
    fp->private_data = ioctl_data;
    return 0;
}
static struct file_operations fops ={
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    .owner = THIS_MODULE,
#endif
    .open = test_ioctl_open,
    .release = test_ioctl_close,
    .read = test_ioctl_read,
    .unlocked_ioctl = test_ioctl_ioctl,
};

static int __init ioctl_init(void){
    dev_t dev;
    int alloc_ret =-1;
    int cdev_ret = -1;
    alloc_ret = alloc_chrdev_region(&dev, 0, num_of_dev, DRIVER_NAME); /*allocates a range of device numbers for the character device*/
    if(alloc_ret) goto error;

    test_ioctl_major = MAJOR(dev); /*find major num*/
    cdev_init(&test_ioctl_cdev, &fops);
    cdev_ret = cdev_add(&test_ioctl_cdev, dev, num_of_dev); /*Adds the character device to the system, making it visible to user-space.*/
    if (cdev_ret) goto error;
    
        pr_alert("%s driver(major: %d) installed.\n", DRIVER_NAME,test_ioctl_major);
    return 0;
    
    error:
        if(!cdev_ret) cdev_del(&test_ioctl_cdev);
        if (!alloc_ret) unregister_chrdev_region(dev, num_of_dev);
        return -1;
}

static void __exit ioctl_exit(void){
    dev_t dev = MKDEV(test_ioctl_major, 0);
    cdev_del(&test_ioctl_cdev);
    unregister_chrdev_region(dev, num_of_dev);
    pr_alert("%s driver removed.\n", DRIVER_NAME);
}
module_init(ioctl_init);
module_exit(ioctl_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple character device driver to demonstrate ioctl functionality.");

