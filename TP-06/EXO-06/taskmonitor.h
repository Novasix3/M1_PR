#ifndef __TASKMONITOR_HEADER__
#define __TASKMONITOR_HEADER__

#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>

struct task_monitor{
	struct pid *p;
	struct task_struct *proc;
};

struct task_sample {
	int pid;
	u64 utime;
	u64 stime;
};

static ssize_t device_write(struct file *file, char __user * buffer,
			   size_t length);
static ssize_t device_read(struct file *file, char __user * buffer,
			   size_t length);
static long hello_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static bool get_sample(struct task_monitor *tm, struct task_sample *sample);
static void print_stats(struct task_sample *sampler);
static int monitor_fn(void *data);

static int monitor_pid(pid_t p);

static int __init taskmonitor_init(void);
static void __exit taskmonitor_exit(void);

#endif