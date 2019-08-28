#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/timekeeping.h>

MODULE_DESCRIPTION("Module \"update_sb\" for linux kernel");
MODULE_AUTHOR("Yoann Valeri, M1 SFPN");
MODULE_LICENSE("GPL");

static char *type = "ext4";
module_param(type, charp, 0660);
MODULE_PARM_DESC(type, "Type of super block to print out");

static void print_bloc(struct super_block *s, void *arg)
{
	ktime_t temp = ktime_get();
	printk("uuid=%pUb, type=%s, time=%lld.%lld\n", &s->s_uuid, 
		s->s_type->name, ktime_to_ms(temp - s->s_time), ktime_to_us(temp - s->s_time));
	s->s_time = temp;
}

static int __init hello_init(void)
{
	struct file_system_type *temp = get_fs_type(type);
	iterate_supers_type(temp, print_bloc, NULL);
	put_filesystem(temp);
	return 0;
}

module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("done ! \n");
}

module_exit(hello_exit);
