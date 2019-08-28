#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include<linux/uaccess.h>
#include <linux/ioctl.h>

#include "request.h"
#include "taskmonitor.h"

MODULE_DESCRIPTION("A taskmonitor kernel module");
MODULE_AUTHOR("Yoann Valeri <yoann.valeri@etu.upmc.fr>");
MODULE_LICENSE("GPL");

static int major;
static struct file_operations* fops;
static char* hello;

static int target = 2;
module_param(target, int, 0660);

static struct task_monitor *monitored = NULL;
static struct task_struct *monitor_thread = NULL;

static unsigned interval = 10; /* interval between reports in seconds */
static unsigned frequency = HZ; /* samples frequency */

static char taskmonitor[PAGE_SIZE];

static struct task_sample *sampler;


static ssize_t device_write(struct file *file, char __user * buffer,
			   size_t length)
{
	int i;
	char* temp = kzalloc(length, GFP_KERNEL);

	for (i = 0; i < length && i < PAGE_SIZE; i++)
		get_user(*(temp + i), buffer + i);

	scnprintf(taskmonitor, PAGE_SIZE, "Hello %s !\n", temp);
	return i;
}

static ssize_t device_read(struct file *file, char __user * buffer,
			   size_t length)
{
	int bytes_read = 0;
	if (*taskmonitor == 0)
		return 0;

	while (length && *taskmonitor) {
		put_user(*(taskmonitor + (bytes_read++)), buffer++);
		length--;
	}
	return bytes_read;
}

static long hello_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int i = 0;
	//char ch = 0;
	//char* temp = NULL;

        switch(cmd) {
                case GET_SAMPLE:
			i = device_read(file, (char *)arg, 99);
			put_user('\0', (char *)arg + i);
                        break;
                case TASKMON_STOP:
			pr_warn("stop demandé\n");
			if(monitor_thread){
				kthread_stop(monitor_thread);
				monitor_thread = NULL;
				pr_warn("stop fait\n");
			}else{
				pr_warn("thread déjà stoppé\n");
			}
			break;
                case TASKMON_START:
			pr_warn("start demandé\n");
			if(!monitor_thread){
				monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
				pr_warn("start fait\n");
			}else{
				pr_warn("thread déjà lancé\n");
			}
			break;
		case TASKMON_SET_PID:
			pr_warn("changement de pid fait : target = %d, to ", target);
			kthread_stop(monitor_thread);
			monitor_thread = NULL;
			copy_from_user(&target ,(int*) arg, sizeof(target));
			monitor_pid(target);
			monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
			pr_warn("%d\n", target);
			break;
                default:
                	return -ENOTTY;
        }
        return 0;
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
	scnprintf(taskmonitor, PAGE_SIZE, "Pid = %d, utime = %lld, stime = %lld.",
		sampler->pid, sampler->utime, sampler->stime);
	
	//pr_info("%s", taskmonitor);
}

static int monitor_fn(void *data)
{
	int n = 0;

	monitored->proc = get_pid_task(monitored->p, PIDTYPE_PID);
	sampler = kzalloc(sizeof(*sampler), GFP_KERNEL);
	pr_warn("début du thread\n");

	while (!kthread_should_stop() && get_sample(monitored, sampler)) {

		if (++n % (interval * (HZ / frequency)) == 0){
			print_stats(sampler);
		}

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(frequency);
	}
	pr_warn("bientôt la fin du thread\n");

	put_task_struct(monitored->proc);
	put_pid(monitored->p);
	kfree(monitored);
	kfree(sampler);
	pr_warn("fin du thread\n");

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

	monitor_thread = kthread_run(monitor_fn, NULL, "my_taskmonitor");
	if(!monitor_thread)
		goto ERROR2;

	hello = kzalloc(PAGE_SIZE, GFP_KERNEL);
	scnprintf(hello, PAGE_SIZE, "taskmonitor module\n");

	fops = kzalloc(sizeof(*fops), GFP_KERNEL);
	fops->unlocked_ioctl = *hello_ioctl;

	major = register_chrdev(0, "taskmonitor", fops);

	//pr_warn("fops = %p, unlocked = %p\n", fops->unlocked_ioctl, *hello_ioctl);

	if(!major)
		goto ERROR3;
	
	pr_warn("module taskmonitor succesfully loaded, with major number = %d\n", major);
	return 0;

ERROR3:
	kfree(fops);
	kfree(hello);

ERROR2:
	//pr_warn("fail du sysfs !\n");
	put_pid(monitored->p);
	kfree(monitored);

ERROR1:
	pr_warn("failed to load module taskmonitor\n");
	return 1;
}

static void __exit taskmonitor_exit(void)
{
	kfree(fops);
	kfree(hello);
	if(major)
		unregister_chrdev(major, "taskmonitor");

	if (monitor_thread){
		//pr_warn("stop du thread !\n");
		kthread_stop(monitor_thread);
		//pr_warn("sortie du sysfs !\n");
	}
	pr_warn("module taskmonitor succesfully unloaded\n");
}

module_init(taskmonitor_init);
module_exit(taskmonitor_exit);