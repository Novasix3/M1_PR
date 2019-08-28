#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>

MODULE_AUTHOR("VALERI Yoann");
MODULE_LICENSE("GPL");

static int target = 2;
module_param(target, int, 0660);

struct task_monitor{
	struct pid *p;
	struct task_struct *proc;
};

static struct task_monitor *monitored;
static struct task_struct *monitor_thread;

static unsigned interval = 10; /* interval between reports in seconds */
static unsigned frequency = HZ; /* samples frequency */

static int monitor_pid(pid_t p)
{
	monitored = kzalloc(sizeof(monitored), GFP_KERNEL);
	monitored->p = find_get_pid(p);

	if(monitored->p)
		return 1;

	return 0;
}

static void print_stats(void)
{
	printk(KERN_INFO "It's alive ! It's alive !!!\n");
	printk(KERN_INFO "Pid = %d, utime = %lld, stime = %lld.\n", 
		monitored->p->numbers[monitored->p->level].nr, 
		monitored->proc->utime, monitored->proc->stime);
}

static int monitor_fn(void *data)
{
	int n = 0;

	monitored->proc = get_pid_task(monitored->p, PIDTYPE_PID);

	while (pid_alive(monitored->proc) && !kthread_should_stop()) {

		if (++n % (interval * (HZ / frequency)) == 0){
			print_stats();
		}

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(frequency);
	}

	put_task_struct(monitored->proc);

	return 0;
}

static int __init taskmonitor_init(void)
{
	if(monitor_pid(target)){
		pr_warn("module successfully loaded !\n");
		monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
	}else{
		pr_warn("failed to load module !\n");
	}
	return 0;
}

static void __exit taskmonitor_exit(void)
{
	if (monitor_thread)
		kthread_stop(monitor_thread);
	
	put_pid(monitored->p);
	kfree(monitored);
}

module_init(taskmonitor_init);
module_exit(taskmonitor_exit);
