#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include<linux/uaccess.h>
#include <linux/ioctl.h>

#include "request.h"

MODULE_DESCRIPTION("A helloioctl kernel module");
MODULE_AUTHOR("Yoann Valeri <yoann.valeri@etu.upmc.fr>");
MODULE_LICENSE("GPL");

static int major;
static struct file_operations* fops;
static char* hello;


static ssize_t device_write(struct file *file, char __user * buffer,
			   size_t length)
{
	int i;
	char* temp = kzalloc(length, GFP_KERNEL);

	for (i = 0; i < length && i < PAGE_SIZE; i++)
		get_user(*(temp + i), buffer + i);

	scnprintf(hello, PAGE_SIZE, "Hello %s !\n", temp);
	return i;
}



static ssize_t device_read(struct file *file, char __user * buffer,
			   size_t length)
{
	int bytes_read = 0;
	if (*hello == 0)
		return 0;

	while (length && *hello) {
		put_user(*(hello + (bytes_read++)), buffer++);
		length--;
	}
	return bytes_read;
}

static long hello_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int i = 0;
	char ch = 0;
	char* temp = NULL;

        switch(cmd) {
                case HELLO:
			i = device_read(file, (char *)arg, 99);
			put_user('\0', (char *)arg + i);
                        break;
                case WHO:
			temp = (char *)arg;
			get_user(ch, temp);
			for (i = 0; ch && i < PAGE_SIZE; i++, temp++)
				get_user(ch, temp);

			device_write(file, (char *)arg, i);
			break;
                default:
                	return -ENOTTY;
        }
        return 0;
}

static int __init hello_init(void)
{
	hello = kzalloc(PAGE_SIZE, GFP_KERNEL);
	scnprintf(hello, PAGE_SIZE, "Hello ioctl !\n");

	fops = kzalloc(sizeof(*fops), GFP_KERNEL);
	fops->unlocked_ioctl = *hello_ioctl;

	major = register_chrdev(0, "hello", fops);

	pr_warn("fops = %p, unlocked = %p\n", fops->unlocked_ioctl, *hello_ioctl);

	if(major)
		pr_warn("module helloioctl succesfully loaded, with major number = %d\n", major);
	else{
		pr_warn("failed to load module helloioctl");
	}
	return 0;
}

static void __exit hello_exit(void)
{
	kfree(fops);
	kfree(hello);
	if(major)
		unregister_chrdev(major, "hello");
	pr_warn("module helloioctl succesfully unloaded");
}

module_init(hello_init);
module_exit(hello_exit);
