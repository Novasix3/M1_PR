#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/utsname.h>

MODULE_DESCRIPTION("Module \"uname\" for linux kernel, change the uname");
MODULE_AUTHOR("Yoann Valeri, M1 SFPN");
MODULE_LICENSE("GPL");

static char old[__NEW_UTS_LEN + 1];

static int __init hello_init(void)
{
	memcpy(&old, &init_uts_ns.name.release, __NEW_UTS_LEN + 1);
	memcpy(&init_uts_ns.name.release, "It's a me, Mario !", 19);
	return 0;

}

module_init(hello_init);

static void __exit hello_exit(void)
{
	memcpy(&init_uts_ns.name.release, &old, __NEW_UTS_LEN + 1);
}

module_exit(hello_exit);
