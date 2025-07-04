#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static void __init start_half(void){
    per_info("the start of a kernel module\n");
}

module_init(start_half);