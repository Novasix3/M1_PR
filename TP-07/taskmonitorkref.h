#ifndef __TASKMONITORKREF_HEADER__
#define __TASKMONITORKREF_HEADER__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/sysfs.h>
#include <linux/shrinker.h>
#include <linux/mempool.h>
#include <linux/kref.h>

MODULE_DESCRIPTION ("Module de monitoring d'un PID");
MODULE_AUTHOR ("VALERI Yoann");
MODULE_LICENSE ("GPL");

struct task_monitor
{
  struct list_head head;
  int nb_samples;
  struct mutex m;
  struct pid *pid;
  struct task_struct *proc;
};

struct task_sample
{
  struct list_head list;
  struct kref ref;
  u64 utime;
  u64 stime;
};

//sampler->total = task->mm->total_vm
//sampler->stack = task->mm->stack_vm

static ssize_t taskmonitor_show (struct kobject *kobj,
				 struct kobj_attribute *attr, char *buf);
static ssize_t taskmonitor_store (struct kobject *kobj,
				  struct kobj_attribute *attr,
				  const char *buf, size_t count);

static void free_task_sample(struct kref *kref);

static unsigned long task_scan_objects (struct shrinker *shinker,
					struct shrink_control *sc);
static unsigned long task_count_objects (struct shrinker *shinker,
					 struct shrink_control *sc);

static bool get_sample (struct task_sample *sample);
static bool save_sample (void);

static int monitor_fn (void *data);
static int monitor_pid (pid_t p);

static int __init taskmonitor_init (void);
module_init (taskmonitor_init);

static void __exit taskmonitor_exit (void);
module_exit (taskmonitor_exit);

static void free_monitored(void);

#endif