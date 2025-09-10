#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_DESCRIPTION("Memory processing");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");

struct task_info {
	pid_t pid;
	unsigned long timestamp;
};

static struct task_info *ti_curr, *ti_parent, *ti_nxt, *ti_nxt_nxt;

static struct task_info *task_info_alloc(int pid)
{
	struct task_info *ti;

	
	ti = kmalloc(sizeof(*ti), GFP_KERNEL);
	if(ti){
		ti ->pid = pid;
		ti->timestamp = jiffies; /*counts the number of timer interrupts*/
	}
	return ti;
}

static int memory_init(void)
{
	struct task_struct *p = current;
	ti_curr = task_info_alloc(p->pid);
	if(p->parent) ti_parent = task_info_alloc(p->parent->pid);
	else{
		ti_parent = NULL;
		pr_info("p->parent == NULL");
	}
	p = next_task(p);
	if(p) {
		ti_nxt = task_info_alloc(p->pid);
		p = next_task(p);
		if (p) ti_nxt_nxt = task_info_alloc(p->pid);
		else pr_info("ti_nxt_nxt == NULL");
	}
	else{
		ti_nxt =NULL;
		ti_nxt_nxt =NULL;
		pr_info("ti_nxt == NULL");
	}
	
	return 0;
}

static void memory_exit(void)
{
	pr_info("current task pid: %i task timestamp %lu\n ", ti_curr->pid, ti_curr->timestamp);
	if(ti_parent) pr_info("parent task pid: %i task timestamp %lu\n ", ti_parent->pid, ti_parent->timestamp);
	if(ti_nxt) pr_info("next task pid: %i task timestamp %lu\n ", ti_nxt->pid, ti_nxt->timestamp);
	if(ti_nxt_nxt) pr_info("next next task pid: %i task timestamp %lu\n ", ti_nxt_nxt->pid, ti_nxt_nxt->timestamp);
	kfree(ti_curr);
	kfree(ti_parent);
	kfree(ti_nxt);
	kfree(ti_nxt_nxt);
}

module_init(memory_init);
module_exit(memory_exit);
