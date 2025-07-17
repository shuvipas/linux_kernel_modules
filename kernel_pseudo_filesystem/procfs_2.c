/*
    insmod
    show the last 10 kernel log messages:
    sudo dmesg -t | tail -10
    see if the file is here:
    ls /proc
    read file:
    cat /proc/buffer1k 
    write to file:
    echo " shuvi's test write to procfs" | sudo tee /proc/buffer1k 
*/

#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>
#include<linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "buffer1k"

static struct proc_dir_entry *our_proc_file;

static char procfs_buffer[PROCFS_MAX_SIZE];

static unsigned long procfs_buffer_size =0; 

/*user wants to read so the kernel needs to write, 
* i.e. the functions name is from the useres point of view
*/
static ssize_t procfile_read(struct file* file_pointer,
                            char __user* buffer, size_t buffer_length, loff_t* offset){
    char s[13] = "HelloWorld!\n";
    int len = sizeof(s);
    ssize_t ret =len;

    if (*offset >= len || copy_to_user(buffer, s, len)) {
    pr_info("copy_to_user failed\n");
    ret = 0; /*if the ret val is never null so it will get called endlesly  */
    } else {
    pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name);
    *offset += len;
    }
    return ret; 
}

static ssize_t procfile_write(struct file* file_pointer,
                            const char __user* buffer, size_t buffer_length, loff_t* offset){
    procfs_buffer_size = buffer_length; /*how much the user is triyng to write*/
    if (procfs_buffer_size >= PROCFS_MAX_SIZE){
        procfs_buffer_size = PROCFS_MAX_SIZE -1;
    }
    if(copy_from_user(procfs_buffer,buffer, procfs_buffer_size)){
        return -EFAULT;
    }
    procfs_buffer[procfs_buffer_size] = '\0';
    *offset  += procfs_buffer_size;
    pr_info("procsfile write%s\n", procfs_buffer);

    return procfs_buffer_size;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read =procfile_read,
    .proc_write = procfile_write,
};
#else 
static const struct file_operations procfile_ops = {
    .read = procfile_read,
    .write = procfile_write,
};
#endif

static int __init procfs_2_init(void){
    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops);
    if(our_proc_file == NULL){
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }
    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit procfs_2_exit(void){
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(procfs_2_init);
module_exit(procfs_2_exit);
MODULE_LICENSE("GPL");

