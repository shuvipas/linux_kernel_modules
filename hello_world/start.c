#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init start_half(void){
    pr_info("the start of a kernel module\n");
    return 0;
}

module_init(start_half);