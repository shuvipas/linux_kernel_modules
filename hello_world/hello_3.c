#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

static int hello_3_data --initdata =3;


static int __int hello_2_init(void){
    pr_info("hi world num: data: %d\n", hello_3_data);
    return 0;
}

static void __exit hello_2_exit(void){
    pr_info("bye world num 3\n");
}

module_init(hello_3_init);
module_exit(hello_3_exit);

MODULE_LICENSE("GPL");