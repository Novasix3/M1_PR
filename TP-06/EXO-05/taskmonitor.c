#include "taskmonitor.h"

MODULE_AUTHOR("VALERI Yoann");
MODULE_LICENSE("GPL");

static int target = 2;
module_param(target, int, 0660);

static struct task_monitor *monitored = NULL;
static struct task_struct *monitor_thread = NULL;

static unsigned interval = 10; /* interval between reports in seconds */
static unsigned frequency = HZ; /* samples frequency */

static char taskmonitor[PAGE_SIZE];

static struct kobj_attribute taskmonitor_attribute =
	__ATTR(taskmonitor, 0664, taskmonitor_show, taskmonitor_store);

static struct attribute *taskmonitor_attrs = &taskmonitor_attribute.attr;

static struct task_sample *sampler;



static ssize_t taskmonitor_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s", taskmonitor);
}

static ssize_t taskmonitor_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	if(!strcmp(buf, "stop") && monitor_thread){
		pr_warn("stop demandé\n");
		kthread_stop(monitor_thread);
		pr_warn("stop fait\n");
	}
	if(!strcmp(buf, "start") && !monitor_thread){
		pr_warn("start demandé\n");
		monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
		pr_warn("start fait\n");
	}
	return count;
}

static bool get_sample(struct task_monitor *tm, struct task_sample *sample)
{
	sample->utime = tm->proc->utime;
	sample->stime = tm->proc->stime;
	sample->pid = tm->p->numbers[tm->p->level].nr;

	return pid_alive(tm->proc) == 1;
}

static void print_stats(struct task_sample *sampler)
{
	scnprintf(taskmonitor, PAGE_SIZE, "Pid = %d, utime = %lld, stime = %lld.\n",
		sampler->pid, sampler->utime, sampler->stime);
	
	//pr_info("%s", taskmonitor);
}

static int monitor_fn(void *data)
{
	int n = 0;

	monitored->proc = get_pid_task(monitored->p, PIDTYPE_PID);
	sampler = kzalloc(sizeof(*sampler), GFP_KERNEL);

	while (!kthread_should_stop() && get_sample(monitored, sampler)) {

		if (++n % (interval * (HZ / frequency)) == 0){
			print_stats(sampler);
		}

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(frequency);
	}

	put_task_struct(monitored->proc);
	kfree(sampler);

	return 0;
}

static int monitor_pid(pid_t p)
{
	monitored = kzalloc(sizeof(monitored), GFP_KERNEL);
	monitored->p = find_get_pid(p);

	if(monitored->p)
		return 1;

	kfree(monitored);
	return 0;
}

static int __init taskmonitor_init(void)
{
	if(!monitor_pid(target))
		goto ERROR1;

	scnprintf(taskmonitor, PAGE_SIZE, "taskmonitor");
	if(sysfs_create_file(kernel_kobj, taskmonitor_attrs))
		goto ERROR2;

	monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
	if(!monitor_thread)
		goto ERROR3;

	pr_warn("module successfully loaded !\n");
	return 0;

ERROR3:
	//pr_warn("fail du thread !\n");
	sysfs_remove_file(kernel_kobj, taskmonitor_attrs);

ERROR2:
	//pr_warn("fail du sysfs !\n");
	put_pid(monitored->p);
	kfree(monitored);

ERROR1:
	//pr_warn("fail de monitor_pid !\n");
	return 1;
}

static void __exit taskmonitor_exit(void)
{
	if (monitor_thread){
		//pr_warn("stop du thread !\n");
		kthread_stop(monitor_thread);
		//pr_warn("sortie du sysfs !\n");
		sysfs_remove_file(kernel_kobj, taskmonitor_attrs);
	}
	
	if(monitored){
		//pr_warn("free du monitored = %p!\n", monitored);
		if(monitored->p){
			//pr_warn("put du monitored->p = %p!\n", monitored->p);
			put_pid(monitored->p);
		}
		kfree(monitored);
	}
}

module_init(taskmonitor_init);
module_exit(taskmonitor_exit);