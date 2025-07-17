/*sysfs allows you to interact with the running kernel from userspace by reading or
setting variables inside of modules.
 insmod
 cat /sys/kernel/mymodule/myvar
 echo "32" | sudo tee /sys/kernel/mymodule/myvar

*/
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

static struct kobject *mymodule;
static int myvar =0;

static ssize_t myvar_show(struct kobject* kobj, struct kobj_attribute *attr, 
                            char* buf)
{
    return sprintf(buf, "%d\n", myvar);
}

static ssize_t myvar_store(struct kobject* kobj, struct kobj_attribute *attr, 
                            const char* buf, size_t count)
{
    sscanf(buf, "%d", &myvar);
    return count;
}

static struct kobj_attribute myvar_attribute = 
        __ATTR(myvar, 0660, myvar_show, myvar_store);

static int __init mymodule_init(void){
    int error =0;
    pr_info("mymodule: initialized\n");
    mymodule = kobject_create_and_add("mymodule", kernel_kobj);
    if (!mymodule)
        return -ENOMEM;

    error = sysfs_create_file(mymodule, &myvar_attribute.attr);
    if (error) {
        kobject_put(mymodule);
        pr_info("failed to create the myvariable file in /sys/kernel/mymodule\n");
    }
    return error;
}

static void __exit mymodule_exit(void){
    pr_info("mymodule: Exit success\n");
    kobject_put(mymodule);
}

module_init(mymodule_init);
module_exit(mymodule_exit);
MODULE_DESCRIPTION("Simple sysfs module exposing 'myvar' under /sys/kernel/mymodule");
MODULE_LICENSE("GPL");