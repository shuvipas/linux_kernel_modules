#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/sched.h>
#include<linux/uaccess.h>
#include<linux/version.h>

#if LINUX_VERSION_CODE>= KERNEL_VERSION(5,10,0)
#include<linux/minmax.h>
#endif

#if LINUX_VERSION_CODE>= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 2048UL
#define PROCFS_ENTRY_FILENAME "buffer2k"

static struct proc_dir_entry *our_proc_file;
static char procs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size =0;

static ssize_t procfs_read( struct file *filep, char __user *buffer, 
                            size_t length, loff_t *offset)
{
    /*already read or buffer empty*/
    if(*offset || procfs_buffer_size ==0){
        pr_debug("procfs_read: END\n");
        *offset =0;
        return 0;
    }
    procfs_buffer_size = min(procfs_buffer_size,length);
    if(copy_to_user(buffer, procs_buffer, procfs_buffer_size)){
        return -EFAULT;
    }
    *offset += procfs_buffer_size;
    pr_debug("procfs_read: read %lu bytes\n", procfs_buffer_size);
    return procfs_buffer_size;
}

static ssize_t procfs_write(struct file *filep, const char __user *buffer, 
                            size_t length, loff_t *offset)
{

    procfs_buffer_size = min(PROCFS_MAX_SIZE,length);
    if(copy_to_user(procs_buffer, buffer, procfs_buffer_size)){
        return -EFAULT;
    }    
    *offset += procfs_buffer_size;
    pr_debug("procfs_write: write %lu bytes\n", procfs_buffer_size);
    return procfs_buffer_size;
}
/*Prevents the module from being unloaded while it's still in use.
prevets Kernel crash (dangling function pointers )
args are not used but Linux kernel requires the open/close function signatures to match
*/
static int procfs_open(struct inode *inode, struct file *file){
    try_module_get(THIS_MODULE);
    return 0;
}
static int procfs_close(struct inode *inode, struct file *file){
    try_module_get(THIS_MODULE);
    return 0;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read =procfs_read,
    .proc_write = procfs_write,
    .proc_open = procfs_open,
    .proc_release = procfs_close,
};
#else 
static const struct file_operations proc_file_fops = {
    .read = procfs_read,
    .write = procfs_write,
    .open = procfs_open,
    .release = procfs_close,
};
#endif

static int __init procfs_3_init(void){
    our_proc_file = proc_create(PROCFS_ENTRY_FILENAME, 0644,NULL, &proc_file_fops);
    if(!our_proc_file){
        pr_debug("Error: Could not initialize /proc/%s\n",PROCFS_ENTRY_FILENAME);
        return -ENOMEM;
    }
    proc_set_size(our_proc_file,80);
    /*Sets the ownership of the /proc file to root:root*/
    proc_set_user(our_proc_file, GLOBAL_ROOT_UID,GLOBAL_ROOT_GID);
    pr_debug("/proc/%s created\n", PROCFS_ENTRY_FILENAME);
    return 0;
}
static void __exit procfs_3_exit(void){
    remove_proc_entry(PROCFS_ENTRY_FILENAME,NULL);
    pr_debug("/proc/%s removed\n", PROCFS_ENTRY_FILENAME);
}
module_init(procfs_3_init);
module_exit(procfs_3_exit);
MODULE_LICENSE("GPL");
