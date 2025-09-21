/* kthread_fixed.c — fixed kthread example (minimal) */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/atomic.h>
#include <linux/delay.h>

MODULE_DESCRIPTION("Simple kernel thread (fixed)");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");

static struct task_struct *my_thread;
static wait_queue_head_t wq_stop_thread;
static atomic_t flag_stop_thread;

/* Thread function */
static int my_thread_fn(void *data)
{
	pr_info("my_thread_fn: started, pid=%d (%s)\n", current->pid, current->comm);

	/* Wait until flag_stop_thread != 0 or thread should be stopped */
	wait_event_interruptible(wq_stop_thread,
		atomic_read(&flag_stop_thread) != 0 || kthread_should_stop());

	pr_info("my_thread_fn: wakeup: flag=%d should_stop=%d\n",
		atomic_read(&flag_stop_thread), kthread_should_stop());

	/* Do any cleanup or final work here... */

	pr_info("my_thread_fn: exiting\n");
	return 0; 
}

/* Module init */
static int __init my_kthread_init(void)
{
	pr_info("my_kthread_init: Init module\n");

	init_waitqueue_head(&wq_stop_thread);
	atomic_set(&flag_stop_thread, 0);

	my_thread = kthread_run(my_thread_fn, NULL, "mykthread");
	if (IS_ERR(my_thread)) {
		pr_err("my_kthread_init: kthread_run failed\n");
		my_thread = NULL;
		return PTR_ERR(my_thread);
	}

	pr_info("my_kthread_init: thread started (pid %d)\n", my_thread->pid);
	return 0;
}


static void __exit my_kthread_exit(void)
{
	int ret;

	pr_info("my_kthread_exit: stopping thread\n");

	
	atomic_set(&flag_stop_thread, 1);
	wake_up_interruptible(&wq_stop_thread);

	
	if (my_thread) {
		ret = kthread_stop(my_thread);
		pr_info("my_kthread_exit: kthread_stop returned %d\n", ret);
	}

	pr_info("my_kthread_exit: Exit module\n");
}

module_init(my_kthread_init);
module_exit(my_kthread_exit);
