#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/sched/signal.h>

MODULE_DESCRIPTION("kernel list processing with synchronization");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");

struct task_info {
	pid_t pid;
	unsigned long timestamp;
	atomic_t count;
	struct list_head list;
};

static struct list_head head;

DEFINE_SPINLOCK(lock);

void task_info_add_for_current(void);
EXPORT_SYMBOL(task_info_add_for_current);

void task_info_print_list(const char *msg);
EXPORT_SYMBOL(task_info_print_list);

void task_info_remove_expired(void);
EXPORT_SYMBOL(task_info_remove_expired);

static struct task_info *task_info_alloc(int pid)
{
	struct task_info *ti;

	ti = kmalloc(sizeof(*ti), GFP_KERNEL);
	if (ti){
	ti->pid = pid;
	ti->timestamp = jiffies;
	atomic_set(&ti->count, 0);
	}
	return ti;
}

static struct task_info *task_info_find_pid(int pid)
{
	struct list_head *p;
	struct task_info *ti;

	list_for_each(p, &head) {
		ti = list_entry(p, struct task_info, list);
		if (ti->pid == pid) {
			return ti;
		}
	}

	return NULL;
}

static void task_info_add_to_list(int pid)
{
	struct task_info *ti;
	
	spin_lock(&lock);
	ti = task_info_find_pid(pid);
	if (ti != NULL) {
		ti->timestamp = jiffies;
		atomic_inc(&ti->count);
		spin_unlock(&lock);
		return;
	}
	spin_unlock(&lock);

	ti = task_info_alloc(pid);
	spin_lock(&lock);
	list_add(&ti->list, &head);
	spin_unlock(&lock);
}

void task_info_add_for_current(void)
{
	task_info_add_to_list(current->pid);
	task_info_add_to_list(current->parent->pid);
	task_info_add_to_list(next_task(current)->pid);
	task_info_add_to_list(next_task(next_task(current))->pid);
}


void task_info_print_list(const char *msg)
{
	struct list_head *p;
	struct task_info *ti;

	pr_info("%s: [ ", msg);

	spin_lock(&lock);
	list_for_each(p, &head) {
		ti = list_entry(p, struct task_info, list);
		pr_info("(%d, %lu, %d) ", ti->pid, ti->timestamp, atomic_read(&ti->count));
	}
	spin_unlock(&lock);
	pr_info("]\n");
}


void task_info_remove_expired(void)
{
	struct list_head *p, *q;
	struct task_info *ti;

	spin_lock(&lock);
	list_for_each_safe(p, q, &head) {
		ti = list_entry(p, struct task_info, list);
		if (jiffies - ti->timestamp > 3 * HZ && atomic_read(&ti->count) < 5) {
			list_del(p);
			kfree(ti);
		}
	}
	spin_unlock(&lock);
}


static void task_info_purge_list(void)
{
	struct list_head *p, *q;
	struct task_info *ti;

	spin_lock(&lock);
	list_for_each_safe(p, q, &head) {
		ti = list_entry(p, struct task_info, list);
		list_del(p);
		kfree(ti);
	}
	spin_unlock(&lock);
}

static int __init list_sync_init(void)
{
	INIT_LIST_HEAD(&head);

	task_info_add_for_current();
	task_info_print_list("after first add");

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(5 * HZ);

	return 0;
}

static void __exit list_sync_exit(void)
{
	/* in order to test task_info_remove_expired we force one proc not expire 
	struct task_info *ti;
	ti = list_entry(head.prev, struct task_info, list);
	atomic_set(&ti->count, 10);
	*/
	task_info_remove_expired();
	task_info_print_list("after removing expired");
	task_info_purge_list();
}

module_init(list_sync_init);
module_exit(list_sync_exit);
