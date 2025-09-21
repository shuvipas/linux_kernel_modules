
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched/task.h>
#include<linux/version.h>

#include "../include/deferred.h"

#define MY_MAJOR		42
#define MY_MINOR		0
#define MODULE_NAME		"deferred"

#define TIMER_TYPE_NONE		-1
#define TIMER_TYPE_SET		0
#define TIMER_TYPE_ALLOC	1
#define TIMER_TYPE_MON		2

MODULE_DESCRIPTION("Deferred work character device");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");

struct mon_proc {
	struct task_struct *task;
	struct list_head list;
};

static struct my_device_data {
	struct cdev cdev;
	struct class* cls;
	struct timer_list timer;
	int flag;
	struct work_struct work;
	struct list_head list;
	spinlock_t lock;
} dev;

static void alloc_io(void)
{
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(5 * HZ);
	pr_info("Yawn! I've been sleeping for 5 seconds.\n");
}

static struct mon_proc *get_proc(pid_t pid)
{
	struct task_struct *task;
	struct mon_proc *p;

	rcu_read_lock();
	task = pid_task(find_vpid(pid), PIDTYPE_PID);
	rcu_read_unlock();
	if (!task)
		return ERR_PTR(-ESRCH);

	p = kmalloc(sizeof(*p), GFP_ATOMIC);
	if (!p)
		return ERR_PTR(-ENOMEM);

	get_task_struct(task);
	p->task = task;

	return p;
}



static void work_handler(struct work_struct *work)
{
	alloc_io();
}

static void timer_handler(struct timer_list *tl)
{
	struct mon_proc *p, *n;
	struct my_device_data *my_data = container_of(tl, struct my_device_data, timer);
	pr_info("timer_handler: pid = %d, comm = %s\n",current->pid, current->comm);	
	switch (my_data->flag)
	{
		case TIMER_TYPE_ALLOC:
			schedule_work(&my_data->work);
			break;

		case TIMER_TYPE_MON:
			spin_lock(&my_data->lock);
			list_for_each_entry_safe(p, n, &my_data->list, list) {
				if (READ_ONCE(p->task->__state) == TASK_DEAD) {
					pr_info("task %s (%d) is dead\n", p->task->comm, p->task->pid);
					put_task_struct(p->task);
					list_del(&p->list);
					kfree(p);
				}
			}
		spin_unlock(&my_data->lock);
		mod_timer(&my_data->timer, jiffies + HZ);
		break;

	}
}

static int deferred_open(struct inode *inode, struct file *file)
{
	struct my_device_data *my_data =
		container_of(inode->i_cdev, struct my_device_data, cdev);
	file->private_data = my_data;
	pr_info("[deferred_open] Device opened\n");
	return 0;
}

static int deferred_release(struct inode *inode, struct file *file)
{
	pr_info("[deferred_release] Device released\n");
	return 0;
}

static long deferred_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct mon_proc *p;
	struct my_device_data *my_data = (struct my_device_data*) file->private_data;

	pr_info("[deferred_ioctl] Command: %s\n", ioctl_command_to_string(cmd));

	switch (cmd) {
		case MY_IOCTL_TIMER_SET:
			my_data->flag = TIMER_TYPE_SET;
			mod_timer(&my_data->timer, jiffies + arg * HZ);
			break;
		case MY_IOCTL_TIMER_CANCEL:
			timer_delete_sync(&my_data->timer);
			break;
		case MY_IOCTL_TIMER_ALLOC:
			my_data->flag = TIMER_TYPE_ALLOC;
			mod_timer(&my_data->timer, jiffies + arg * HZ);
			break;
		case MY_IOCTL_TIMER_MON:
		{
			p = get_proc(arg);
			if (IS_ERR(p))
				return PTR_ERR(p);
			
			spin_lock_bh(&my_data->lock);
			list_add(&p->list, &my_data->list);
			spin_unlock_bh(&my_data->lock);

			my_data->flag = TIMER_TYPE_MON;
			mod_timer(&my_data->timer, jiffies + HZ);
			break;
		}
		default:
			return -ENOTTY;
	}
	return 0;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = deferred_open,
	.release = deferred_release,
	.unlocked_ioctl = deferred_ioctl,
};

static int __init deferred_init(void)
{
	int err;

	pr_info("[deferred_init] Init module\n");
	err = register_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), 1, MODULE_NAME);
	if (err) {
		pr_info("[deffered_init] register_chrdev_region: %d\n", err);
		return err;
	}
	dev.flag = TIMER_TYPE_NONE;
	
	INIT_WORK(&dev.work, work_handler);
	spin_lock_init(&dev.lock);
	INIT_LIST_HEAD(&dev.list);

	cdev_init(&dev.cdev, &my_fops);
	cdev_add(&dev.cdev, MKDEV(MY_MAJOR, MY_MINOR), 1);

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
        dev.cls = class_create(MODULE_NAME);
    #else
        dev.cls = class_create(THIS_MODULE, MODULE_NAME);
    #endif
	device_create(dev.cls, NULL, MKDEV(MY_MAJOR, MY_MINOR), NULL, MODULE_NAME);

	timer_setup(&dev.timer, timer_handler,0);

	return 0;
}


static void __exit deferred_exit(void)
{
	struct mon_proc *p, *n;

	pr_info("[deferred_exit] Exit module\n" );
	device_destroy(dev.cls, MKDEV(MY_MAJOR, MY_MINOR));
    class_destroy(dev.cls);
	cdev_del(&dev.cdev);
	unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), 1);

	timer_delete_sync(&dev.timer);
	flush_scheduled_work(); /*flushes the entire system workqueue. todo insted use a dedicated workqueue*/

	list_for_each_entry_safe(p, n, &dev.list, list) {
		put_task_struct(p->task);
		list_del(&p->list);
		kfree(p);
	}
}

module_init(deferred_init);
module_exit(deferred_exit);
