#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "./so2_cdev.h"

MODULE_DESCRIPTION("SO2 character device");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");

#define LOG_LEVEL	KERN_INFO

#define MY_MAJOR		42
#define MY_MINOR		0
#define NUM_MINORS		1
#define MODULE_NAME		"so2_cdev"
#define MESSAGE			"hello sp\n"
#define IOCTL_MESSAGE		"Hello ioctl"

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif


struct so2_device_data {
	struct cdev cdev;
	char buffer[BUFSIZ];
	size_t size;
	wait_queue_head_t q;
	int flag;
	atomic_t dev_opend;
};

struct so2_device_data devs[NUM_MINORS];

static int so2_cdev_open(struct inode *inode, struct file *file)
{
	struct so2_device_data *data;

	pr_info("so2_cdev_open\n");

	data = container_of(inode->i_cdev, struct so2_device_data, cdev);
	file->private_data = data;
	if(atomic_cmpxchg(&data->dev_opend, 0,1)){
		pr_info("so2_cdev_open file is already opend ,dev_opend = %d\n ",atomic_read(&data->dev_opend));
		return -EBUSY;
	}

	set_current_state(TASK_INTERRUPTIBLE); /*set task to sleep*/
	schedule_timeout(10 * HZ);

	return 0;
}

static int
so2_cdev_release(struct inode *inode, struct file *file)
{
	pr_info("so2_cdev_release\n");

	struct so2_device_data *data = file->private_data;

	atomic_set(&data->dev_opend,0);
	return 0;
}

static ssize_t 
so2_cdev_read(struct file *file,
		char __user *user_buffer,
		size_t size, loff_t *offset)
{
	struct so2_device_data *data =  file->private_data;
	
	size = min(size , data->size -*offset);
	if(size <= 0)
	{
		pr_info("so2_cdev_read no bytes to read\n");
		return 0;
	}	
	if(copy_to_user(user_buffer, data->buffer + *offset,size))
	{
		return -EFAULT;
	}
	*offset += size;
	return size;
}

static ssize_t
so2_cdev_write(struct file *file,
		const char __user *user_buffer,
		size_t size, loff_t *offset)
{
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;
		size = min(size , data->size -*offset);
	if(size <= 0)
	{
		pr_info("so2_cdev_read no bytes to read\n");
		return 0;
	}	
	if(copy_from_user(data->buffer + *offset,user_buffer,size))
	{
		return -EFAULT;
	}
	*offset += size;
	
	return size;
}

static long
so2_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;
	int ret = 0;
	int remains;

	switch (cmd) {
		case MY_IOCTL_PRINT:
			pr_info("ioctl msg: %s\n", IOCTL_MESSAGE);
			break;
		case MY_IOCTL_SET_BUFFER:
			remains = copy_from_user(data->buffer, (char __user *)arg, BUFFER_SIZE);
			if(remains <=0)
			{
				ret = -EFAULT;
			}
			break;
		case MY_IOCTL_GET_BUFFER:
			if(copy_to_user((char __user *)arg, data->buffer, BUFFER_SIZE))
			{
				ret = -EFAULT;
			}
			break;
		case MY_IOCTL_DOWN:
			data->flag = 0;
			ret = wait_event_interruptible(data->q, data->flag != 0);		
			break;
		case MY_IOCTL_UP:
			data->flag = 1;
			wake_up_interruptible(&data->q);
			break;
		default:
			pr_info("so2_cdev_ioctl ERROR: bad ioctl cmd %d", cmd);
			ret = -EINVAL;
	}
	return ret;
}

static const struct file_operations so2_fops = {
	.owner = THIS_MODULE,
	.open = so2_cdev_open,
	.release = so2_cdev_release,
	.read = so2_cdev_read,
	.write = so2_cdev_write,
	.unlocked_ioctl = so2_cdev_ioctl,
};

static int __init so2_cdev_init(void)
{
	int err;
	int i;

	err = register_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR),NUM_MINORS, MODULE_NAME);
	if(!err){
		pr_info("ERROR: register_chrdev_region did not register device");
		return err;
	}

	for (i = 0; i < NUM_MINORS; i++) {
		memcpy(devs[i].buffer, MESSAGE,sizeof(MESSAGE));
		devs[i].size = sizeof(MESSAGE);
		atomic_set(&devs[i].dev_opend,0);
		init_waitqueue_head(&devs[i].q);
		devs[i].flag =0;
		cdev_init(&devs[i].cdev,&so2_fops);
		cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR,i), 1);
	}
	return 0;
}

static void __exit so2_cdev_exit(void)
{
	int i;

	for (i = 0; i < NUM_MINORS; i++) {
		cdev_del(&devs[i].cdev);
	}
	unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR),NUM_MINORS);
}

module_init(so2_cdev_init);
module_exit(so2_cdev_exit);
