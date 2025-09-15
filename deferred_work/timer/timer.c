
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>


MODULE_DESCRIPTION("Simple kernel timer");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");

#define TIMER_TIMEOUT	1

static struct timer_list timer;
static int counter;
static void timer_handler(struct timer_list *tl)
{
	counter++;
	pr_info("timer_handler after %d secondes\n",counter);
	
	mod_timer(tl, jiffies + TIMER_TIMEOUT * HZ);
}

static int __init timer_init(void)
{
	pr_info("[timer_init] Init module\n");
	counter = 0;
	timer_setup(&timer, timer_handler,0);
	mod_timer(&timer, jiffies + TIMER_TIMEOUT * HZ);

	return 0;
}

static void __exit timer_exit(void)
{
	pr_info("[timer_exit] Exit module\n");
	timer_delete_sync(&timer);
}

module_init(timer_init);
module_exit(timer_exit);
