#include <linux/module.h>
#include <linux/printk.h>

int init_module(void){ // old sythex dosnt run on fedora but runs on fedora
    pr_info("hello world 1\n");
    return 0;
}
void cleanup_module(void){
    pr_info("goodbye world 1\n");

}

MODULE_LICENSE("GPL");
