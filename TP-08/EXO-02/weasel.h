#ifndef __WEASEL_HEADER__
#define __WEASEL_HEADER__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dcache.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>

MODULE_DESCRIPTION ("Module weasel.");
MODULE_AUTHOR ("VALERI Yoann");
MODULE_LICENSE ("GPL");

static int pwd_proc_show(struct seq_file *m, void *v);
static int pwd_proc_open(struct inode *inode, struct file *file);

static int list_proc_show(struct seq_file *m, void *v);
static int list_proc_open(struct inode *inode, struct file *file);

static int weasel_proc_show(struct seq_file *m, void *v);
static int weasel_proc_open(struct inode *inode, struct file *file);
static ssize_t weasel_proc_write(struct file *file,
                const char __user *buffer, size_t count, loff_t *pos);

static void show(void);

static int __init weasel_init (void);
module_init (weasel_init);

static void __exit weasel_exit (void);
module_exit (weasel_exit);

#endif