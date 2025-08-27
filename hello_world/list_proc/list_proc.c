#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include<linux/version.h>
#include <linux/mm.h>


#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
#define HAVE_VMA_FUNC
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,5)
#define HAVE_VMA_ITR
#endif


MODULE_DESCRIPTION("List current processes");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");

static void print_all_processes(void){
	struct task_struct *p = current;
	pr_info("List of all processes\n");
	for_each_process(p){
		pr_info("process name: %s pid: %x\n",p->comm, p->pid);
	}
	
}
static void print_curr_process_info(void){
	struct task_struct *p = current;
	struct vm_area_struct *curr_vma;
	#ifdef HAVE_VMA_ITR
	struct vma_iterator vmi;
	#endif
	pr_info("current process pid: %x  name: %s \n",p->pid, p->comm);
	if(!p->mm){
		pr_info("kernel thread—no user-space VMAs\n");
		return;
	}
	pr_info("Memory Info:\n");
	#ifdef HAVE_VMA_ITR

	vma_iter_init(&vmi, p->mm, 0);  
	for_each_vma(vmi, curr_vma) {
    	pr_info("VMA: start=%lx end=%lx size=%lu bytes\n",curr_vma->vm_start, curr_vma->vm_end,curr_vma->vm_end - curr_vma->vm_start);
	}
	#else
		#ifdef HAVE_VMA_FUNC
			curr_vma = find_vma(p->mm,0);
			while(curr_vma != NULL){
				pr_info("VMA: start %lx end: %lx\n", curr_vma->vm_start, curr_vma->vm_end);
				curr_vma = vma_next(p->mm, curr_vma);
			}
		#else
			for(curr_vma = p->mm->mmap;curr_vma != NULL; curr_vma = curr_vma->vm_next){
				pr_info("VMA: start %lx end: %lx\n", curr_vma->vm_start, curr_vma->vm_end);
			}
		#endif
	#endif	
}
static int my_proc_init(void)
{
	print_curr_process_info();
	print_all_processes();
	
	return 0;
}

static void my_proc_exit(void)
{
	pr_info("removed module: list_proc\n");
}

module_init(my_proc_init);
module_exit(my_proc_exit);
