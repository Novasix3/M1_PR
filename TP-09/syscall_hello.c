#include <linux/printk.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/slab.h>

SYSCALL_DEFINE4(hello, char*, who, int, who_length, char*, buf, int, buf_length)
{
	int ret;

	if( (who_length + 10) > buf_length )
		return 0;

	char *buffer = kzalloc(who_length + 1, GFP_KERNEL);
	copy_from_user(buffer, who, who_length+1);

	char *temp = kzalloc(who_length + 10, GFP_KERNEL);

	scnprintf(temp, who_length + 10, "Hello %s !\n", buffer);

	ret = copy_to_user(buf, temp, who_length + 10);
	kfree(buffer);
	kfree(temp);

	return ret;
}