#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Module \"hello word\" pour noyau linux (with param !)");
MODULE_AUTHOR("Yoann Valeri, M1 SFPN");
MODULE_LICENSE("GPL");

static char *whom = "Yoann";
module_param(whom, charp, 0660);
MODULE_PARM_DESC(whom, "Person to greet when initiating the module.");
static int howmany = 3;
module_param(howmany, int, 0660);
MODULE_PARM_DESC(howmany, "How many times we greet the person.");

static int __init hello_init(void)
{
	int i = 0;

	for (i = 0; i < howmany; ++i)
		pr_info("(%d) Hello, %s !\n", i, whom);
	return 0;
}

module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("Goodbye, %s\n", whom);
}

module_exit(hello_exit);
