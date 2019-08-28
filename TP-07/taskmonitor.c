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

LIST_HEAD(my_head);

static struct shrinker *shrinker;



static ssize_t taskmonitor_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	char* temp = kzalloc(PAGE_SIZE, GFP_KERNEL);
	struct task_sample *k = NULL;
	int n;

	scnprintf(temp, PAGE_SIZE, "\n");

	list_for_each_entry(k, &monitored->head, list) {
		scnprintf(temp, PAGE_SIZE, "%s\nPid = %d, utime = %lld, stime = %lld.", 
			temp, monitored->p->numbers[monitored->p->level].nr, 
			k->utime, k->stime);
	}
	n = scnprintf(buf, PAGE_SIZE, "%s\n", temp);
	kfree(temp);

	return n;
}

static ssize_t taskmonitor_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	if(!strcmp(buf, "stop") && monitor_thread){
		//pr_warn("stop demandé\n");
		kthread_stop(monitor_thread);
		monitor_thread = NULL;
		//pr_warn("stop fait\n");
	}
	if(!strcmp(buf, "start") && !monitor_thread){
		//pr_warn("start demandé\n");
		monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
		//pr_warn("start fait\n");
	}
	return count;
}

static unsigned long task_scan_objects(struct shrinker *shinker, struct shrink_control *sc)
{
	struct task_sample *k, *prev;
	int max = sc->nr_to_scan;

	list_for_each_entry_safe(k, prev, &monitored->head, list) {
		list_del(&k->list);
		kfree(k);
		sc->nr_scanned++;
		if(--max == 0)
			break;
	}

	return (sc->nr_to_scan - max);
}

static unsigned long task_count_objects(struct shrinker *shinker, struct shrink_control *sc)
{
	if(monitored->nbSamples == 0)
		return SHRINK_EMPTY;

	return (monitored->nbSamples < 5)?monitored->nbSamples:5;
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
	struct task_sample* temp;

	pr_warn("on récup le sample test!\n\n\n\n\n");
	temp = get_sample();
	pr_warn("récup du sample test %p!\n\n\n\n\n", temp);
	pr_warn("ajout d'une stat -> lock\n\n\n\n\n");
	mutex_lock(&monitored->m);
	pr_warn("lock fait\n\n\n\n\n");
	list_add_tail(&temp->list, &my_head);
	pr_warn("save sample fait\n\n\n\n\n");
	mutex_unlock(&monitored->m);
	pr_warn("unlock fait\n\n\n\n\n");
	pr_warn("ajout à la fin de la liste de %p!\n\n\n\n\n", temp);
	
	monitored->nbSamples += 1;
}

/*
static char* print_stats(struct task_sample *sampler)
{
	//printk(KERN_INFO "It's alive ! It's alive !!!\n");
	char* temp = kzalloc(PAGE_SIZE, GFP_KERNEL);
	scnprintf(temp, PAGE_SIZE, "Pid = %d, utime = %lld, stime = %lld.\n", 
		monitored->p->numbers[monitored->p->level].nr, 
		sampler->utime, sampler->stime);
	return temp;
}
*/

static int monitor_pid(pid_t p)
{
	struct pid *pid = find_get_pid(p);

	if(!pid)
		return 0;

	pr_warn("module 0 loaded !\n\n\n\n\n");
	monitored = kzalloc(sizeof(monitored), GFP_KERNEL);
	pr_warn("module 0.5 loaded !\n\n\n\n\n");
	monitored->p = pid;
	pr_warn("module 1 loaded !\n\n\n\n\n");
	mutex_init(&monitored->m);
	pr_warn("module 2 loaded !\n\n\n\n\n");
	monitored->nbSamples = 0;

	monitored->head = my_head;

	return 1;
}

static int monitor_fn(void *data)
{
	int n = 0;

	monitored->proc = get_pid_task(monitored->p, PIDTYPE_PID);
	pr_warn("on récup le proc %p!\n\n\n\n\n", monitored->proc);


	while (pid_alive(monitored->proc) && !kthread_should_stop()) {
		pr_warn("wesh %p!\n\n\n\n\n\n", monitored->proc);

		if (++n % (interval * (HZ / frequency)) == 0){
		pr_warn("wesh1 %p!\n\n\n\n\n\n", monitored->proc);
			save_sample();
		}
		pr_warn("wesh2 %p!\n\n\n\n\n\n", monitored->proc);

		set_current_state(TASK_UNINTERRUPTIBLE);
		pr_warn("wesh3 %p!\n\n\n\n\n\n", monitored->proc);
		schedule_timeout(frequency);
		pr_warn("wesh4 %p!\n\n\n\n\n\n", monitored->proc);
	}

	pr_warn("on veut put le proc %p!\n", monitored->proc);
	put_task_struct(monitored->proc);
	pr_warn("put du proc fait %p!\n", monitored->proc);

	return 0;
}

static void define_shrinker()
{
	shrinker = kzalloc(sizeof(*shrinker), GFP_KERNEL);
	shrinker->count_objects = task_count_objects;
	shrinker->scan_objects = task_scan_objects;
	shrinker->flags = 0;
	shrinker->seeks = DEFAULT_SEEKS;

	register_shrinker(shrinker);
}

static void free_shrinker()
{
	unregister_shrinker(shrinker);
	kfree(shrinker);
}

static int __init taskmonitor_init(void)
{
	if(!monitor_pid(target))
		goto ERROR1;
	pr_warn("module 1 loaded !\n");

	scnprintf(taskmonitor, PAGE_SIZE, "taskmonitor");
	if(sysfs_create_file(kernel_kobj, taskmonitor_attrs))
		goto ERROR2;
	pr_warn("module 2 loaded !\n");

	define_shrinker();

	monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
	if(!monitor_thread)
		goto ERROR3;
	pr_warn("module 3 loaded !\n");

	pr_warn("module successfully loaded !\n");
	return 0;

ERROR3:
	//pr_warn("fail du thread !\n");
	sysfs_remove_file(kernel_kobj, taskmonitor_attrs);

ERROR2:
	//pr_warn("fail du sysfs !\n");
	mutex_destroy(&monitored->m);
	put_pid(monitored->p);
	kfree(monitored);

ERROR1:
	//pr_warn("fail de monitor_pid !\n");
	return -1;
}

static void __exit taskmonitor_exit(void)
{
	struct task_sample *k, *prev;

	if (monitor_thread)
		kthread_stop(monitor_thread);

	sysfs_remove_file(kernel_kobj, taskmonitor_attrs);
	
	list_for_each_entry_safe(k, prev, &monitored->head, list) {
		list_del(&k->list);
		kfree(k);
	}
	mutex_destroy(&monitored->m);
	put_pid(monitored->p);

	list_del(&my_head);
	kfree(&my_head);

	free_shrinker();

	kfree(monitored);

}

module_init(taskmonitor_init);
module_exit(taskmonitor_exit);