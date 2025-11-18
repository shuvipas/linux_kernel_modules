#include <linux/module.h>


MODULE_DESCRIPTION("Real-time process resource monitor with power estimation and categorical grouping");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");


struct my_proc{
    struct task_struct *p;

};
// get current power usege from current procces
static void print_curr_power_usg(){

}


static int __init proc_tracker_init(void){

}

static void __exit proc_tracker_exit(void){
    
}



module_init(proc_tracker_init);
module_exit(proc_tracker_exit);