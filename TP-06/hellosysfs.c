#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_AUTHOR("VALERI Yoann");
MODULE_LICENSE("GPL");

static char hello[PAGE_SIZE];

static ssize_t hello_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "Hello %s!\n", hello);
}

static ssize_t hello_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = scnprintf(hello, PAGE_SIZE, "%s", buf);
	if ( ret < 0 )
		return ret;
	return count;
}

static struct kobj_attribute hellosysfs_attribute =
	__ATTR(hello, 0664, hello_show, hello_store);

static struct attribute *hellosys_attrs = &hellosysfs_attribute.attr;

static int __init hello_init(void)
{
	scnprintf(hello, PAGE_SIZE, "sysfs");
	return sysfs_create_file(kernel_kobj, hellosys_attrs);
}

static void __exit hello_exit(void)
{
	sysfs_remove_file(kernel_kobj, hellosys_attrs);
}

module_init(hello_init);
module_exit(hello_exit);
