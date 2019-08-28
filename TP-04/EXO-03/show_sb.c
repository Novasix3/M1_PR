#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>

MODULE_DESCRIPTION("Module \"show_sb\" for linux kernel");
MODULE_AUTHOR("Yoann Valeri, M1 SFPN");
MODULE_LICENSE("GPL");

static void print_bloc(struct super_block *s, void *arg)
{
	printk("uuid=%pUb, type=%s", &s->s_uuid, s->s_id);
}

static int __init hello_init(void)
{
	iterate_supers(print_bloc, NULL);
	return 0;
}

module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("done ! ");
}

module_exit(hello_exit);
