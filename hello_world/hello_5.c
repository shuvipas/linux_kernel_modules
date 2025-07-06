#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/printk.h>
#include<linux/stat.h>

MODULE_LICENSE("GPL");

static short  myshort = 3;
static int myint = 77;
static long mylong = 9999;
static char* mystr = "default for mystring";
static int intarr[2] = {18,19};
static int arr_argc =0;

//permission: S_IRUSR = (R)-read, (W)- write, 
// (USR)- root/user/owener of the file (GRP)- group

module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myshort, "A short integer");  //description for a module parameter (see it with modinfo)
module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(myint, "An integer");
module_param(mylong, long, S_IRUSR);
MODULE_PARM_DESC(mylong, "A long integer");
module_param(mystr, charp, 0000);
MODULE_PARM_DESC(mystr, "A character string");

module_param_array(intarr,int,&arr_argc,0000);
MODULE_PARM_DESC(intarr, "An array of integers");

static int __init hello_5_init(void){
    // the kernel coding style declare all variables at the start of a block
    int i; 

    pr_info("Hello, world 5\n=============\n");
    pr_info("myshort is a short integer: %hd\n", myshort);
    pr_info("myint is an integer: %d\n", myint);
    pr_info("mylong is a long integer: %ld\n", mylong);
    pr_info("mystr is a string: %s\n", mystr);

    for(i=0; i<ARRAY_SIZE(intarr);i++){
        pr_info("intarr[%d] = %d\n", i, intarr[i]);
    }
    pr_info("got %d arguments for myintarray.\n", arr_argc);
    return 0;   
}


static void __exit hello_5_exit(void){
    pr_info("bye bye world 5\n");
}

module_init(hello_5_init);
module_exit(hello_5_exit);