#include "taskmonitor.h"

static int target = 2;
module_param(target, int, 0660);

static struct task_monitor *monitored;
static struct task_struct *monitor_thread;

static unsigned interval = 10; /* interval between reports in seconds */
static unsigned frequency = HZ; /* samples frequency */

static char taskmonitor[PAGE_SIZE];

static struct kobj_attribute taskmonitor_attribute =
	__ATTR(taskmonitor, 0664, taskmonitor_show, taskmonitor_store);

static struct attribute *taskmonitor_attrs = &taskmonitor_attribute.attr;

static ssize_t taskmonitor_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	char* temp = kzalloc(PAGE_SIZE, GFP_KERNEL);
	struct task_sample *k = NULL;

	scnprintf(temp, PAGE_SIZE, "\n");

	list_for_each_entry(k, &monitored->sample->list, list) {
		scnprintf(temp, PAGE_SIZE, "%s\nPid = %d, utime = %lld, stime = %lld.", 
			temp, monitored->p->numbers[monitored->p->level].nr, 
			k->utime, k->stime);
	}
	int n = scnprintf(buf, PAGE_SIZE, "%s\n", temp);
	kfree(temp);
	kfree(k);
	return n;
}

static ssize_t taskmonitor_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	if(!strcmp(buf, "stop") && monitor_thread){
		pr_debug("stop demandé\n");
		kthread_stop(monitor_thread);
		pr_debug("stop fait\n");
	}
	if(!strcmp(buf, "start") && !monitor_thread){
		pr_debug("start demandé\n");
		monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
		pr_debug("start fait\n");
	}
	return count;
}

static int monitor_pid(pid_t p)
{
	struct pid *pid = find_get_pid(p);

	if(!pid)
		return 0;

	monitored = kzalloc(sizeof(monitored), GFP_KERNEL);
	monitored->p = pid;
	mutex_init(&monitored->m);
	pr_debug("module 1 loaded !\n\n\n\n\n");
	monitored->nbSamples = 0;
	struct task_sample* temp = kzalloc(sizeof(*temp), GFP_KERNEL);
	struct list_head *my_head = kzalloc(sizeof(*my_head), GFP_KERNEL);
	temp->list = *my_head;
	INIT_LIST_HEAD(&temp->list);
	monitored->sample = temp;
	monitored->shrinker = kzalloc(sizeof(*monitored->shrinker), GFP_KERNEL);
	shrinker->count_objects = task_count_objects;
	shrinker->scan_objects = task_scan_objects;
	shrinker->flags = 0;
	shrinker->seeks = DEFAULT_SEEKS;
	return 1;
}

static char* print_stats(struct task_sample *sampler)
{
	//printk(KERN_INFO "It's alive ! It's alive !!!\n");
	char* temp = kzalloc(PAGE_SIZE, GFP_KERNEL);
	scnprintf(temp, PAGE_SIZE, "Pid = %d, utime = %lld, stime = %lld.\n", 
		monitored->p->numbers[monitored->p->level].nr, 
		sampler->utime, sampler->stime);
	return temp;
}

static struct task_sample* get_sample(void)
{
	struct task_sample* temp = kzalloc(sizeof(*temp), GFP_KERNEL);
	temp->utime = monitored->proc->utime;
	temp->stime = monitored->proc->stime;
	return temp;
}

static void save_sample(void)
{
	pr_debug("on récup le sample test!\n");
	struct task_sample* temp = get_sample();
	pr_debug("récup du sample test %p!\n", temp);
	pr_debug("on veut ajouter à la fin de la liste %p!\n", temp);
	pr_debug("ajout d'une stat -> lock\n");
	mutex_lock(&monitored->m);
	pr_debug("lock fait\n");
	list_add_tail(&temp->list, &monitored->sample->list);
	pr_debug("save sample fait\n");
	mutex_unlock(&monitored->m);
	pr_debug("unlock fait\n");
	pr_debug("ajout à la fin de la liste de %p!\n", temp);
	
	monitored->nbSamples += 1;
}

static int monitor_fn(void *data)
{
	int n = 0;

	monitored->proc = get_pid_task(monitored->p, PIDTYPE_PID);
	pr_debug("on récup le proc %p!\n", monitored->proc);


	while (pid_alive(monitored->proc) && !kthread_should_stop()) {

		if (++n % (interval * (HZ / frequency)) == 0){
			save_sample();
		}

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(frequency);
	}

	pr_debug("on veut put le proc %p!\n", monitored->proc);
	put_task_struct(monitored->proc);
	pr_debug("put du proc fait %p!\n", monitored->proc);

	return 0;
}

static int __init taskmonitor_init(void)
{
	if(!monitor_pid(target))
		goto ERROR1;
	pr_debug("module 1 loaded !\n");

	scnprintf(taskmonitor, PAGE_SIZE, "taskmonitor");
	if(sysfs_create_file(kernel_kobj, taskmonitor_attrs))
		goto ERROR2;
	pr_debug("module 2 loaded !\n");

	monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
	if(!monitor_thread)
		goto ERROR3;
	pr_debug("module 3 loaded !\n");

	pr_debug("module successfully loaded !\n");
	return 0;

ERROR3:
	//pr_debug("fail du thread !\n");
	sysfs_remove_file(kernel_kobj, taskmonitor_attrs);

ERROR2:
	//pr_debug("fail du sysfs !\n");
	mutex_destroy(&monitored->m);
	put_pid(monitored->p);
	kfree(monitored);

ERROR1:
	//pr_debug("fail de monitor_pid !\n");
	return -1;
}

static void __exit taskmonitor_exit(void)
{
	struct task_sample *k, *prev;

	if (monitor_thread)
		kthread_stop(monitor_thread);

	sysfs_remove_file(kernel_kobj, taskmonitor_attrs);
	
	list_for_each_entry_safe(k, prev, &monitored->sample->list, list) {
		list_del(&k->list);
		kfree(k);
	}
	mutex_destroy(&monitored->m);
	put_pid(monitored->p);
	kfree(monitored->shrinker);
	kfree(monitored->sampler);
	kfree(monitored);

}

module_init(taskmonitor_init);
module_exit(taskmonitor_exit);
