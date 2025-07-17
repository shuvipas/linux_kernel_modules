/* a seq file is like a for-each loop for kernel-space data that you expose in /proc.
It manages the memory, buffering, paging, and user copy, and just asks you:
"What’s the next item? How should I print it?"
*/

#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/seq_file.h>
#include<linux/version.h>

#if LINUX_VERSION_CODE>= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

#define PROC_NAME "iter"

static void *my_seq_start(struct seq_file *s, loff_t *pos){
    static unsigned long counter =0;
    if(*pos == 0){
        return &counter; /*return only for first read*/
    }
    *pos =0;
    return NULL;
}

static void* my_seq_next(struct seq_file *s,void* v, loff_t *pos){
    unsigned long *tmp_v = (unsigned long *)v;
    (*tmp_v)++;
    (*pos)++;
    return NULL;
} 

static void my_seq_stop(struct seq_file* s, void* v){
/*function is called after the sequence ends,
we dont have anything to clean up so its empty
*/
}
static int my_seq_show(struct seq_file* s, void* v){
    loff_t *spos = (loff_t*)v;
    seq_printf(s, "%Ld\n", *spos);
    return 0;
}

static struct seq_operations my_seq_ops ={
    .start= my_seq_start,
    .next = my_seq_next,
    .stop = my_seq_stop,
    .show = my_seq_show,
};

static int my_open(struct inode *inode, struct file *file)
{
return seq_open(file, &my_seq_ops);
};

#ifdef HAVE_PROC_OPS
static const struct proc_ops my_file_ops = {
    .proc_open = my_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = seq_release,
};
#else
static const struct file_operations my_file_ops = {
    .open = my_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};
#endif

static int __init procfs_4_init(void){
    struct proc_dir_entry *entry =  proc_create(PROC_NAME, 0,NULL, &my_file_ops);
    if(!entry){
        pr_debug("Error: Could not initialize /proc/%s\n",PROC_NAME);
        return -ENOMEM;
    }
    return 0;
}
static void __exit procfs_4_exit(void){
    remove_proc_entry(PROC_NAME,NULL);
    pr_debug("/proc/%s removed\n", PROC_NAME);
}
module_init(procfs_4_init);
module_exit(procfs_4_exit);
MODULE_LICENSE("GPL");