#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static void __exit stop_half(void){
    per_info("the end of a kernel module\n");
}

module_exit(stop_half);