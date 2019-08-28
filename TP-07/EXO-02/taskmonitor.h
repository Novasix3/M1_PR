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
#include <linux/list.h>
#include <linux/shrinker.h>

MODULE_AUTHOR("VALERI Yoann");
MODULE_LICENSE("GPL");

struct task_monitor{
	struct list_head head;
	int nbSamples;
	struct mutex m;
	struct pid *p;
	struct task_struct *proc;
	struct shrinker* shrinker;
};

struct task_sample {
	struct list_head list;
	u64 utime;
	u64 stime;
	unsigned long total;
	unsigned long stack;
};

//sampler->total = task->mm->total_vm
//sampler->stack = task->mm->stack_vm

static ssize_t taskmonitor_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf);
static ssize_t taskmonitor_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count);

static unsigned long task_scan_objects(struct shrinker *shinker, struct shrink_control *sc);
static unsigned long task_count_objects(struct shrinker *shinker, struct shrink_control *sc);

static struct task_sample* get_sample(void);
static void save_sample(void);

//static char* print_stats(struct task_sample *sampler);
static int monitor_fn(void *data);
static int monitor_pid(pid_t p);

static void define_shrinker(void);
static void free_shrinker(void);

static int __init taskmonitor_init(void);
static void __exit taskmonitor_exit(void);

#endif