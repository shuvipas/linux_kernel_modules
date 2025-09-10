#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>

MODULE_DESCRIPTION("Sleep while atomic");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

static int sched_spin_init(void)
{
	// spinlock_t lock;

	// spin_lock_init(&lock);
	DEFINE_SPINLOCK(lock);
	spin_lock(&lock);
	/*
	The process sets its state to a sleeping state
	set_current_state(TASK_INTERRUPTIBLE); // BUG: scheduling while atomic: (deadlock and cant wake up)
	 Try to sleep for 5 seconds.
	schedule_timeout(5 * HZ);// BUG: scheduling while atomic: (deadlock and cant wake up)
	 */
	spin_unlock(&lock);

	return 0;
}

static void sched_spin_exit(void)
{
}

module_init(sched_spin_init);
module_exit(sched_spin_exit);
