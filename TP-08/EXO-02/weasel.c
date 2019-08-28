#include "weasel.h"

extern unsigned int d_hash_shift;

static const struct file_operations weasel_proc_fops = {
        .owner          = THIS_MODULE,
        .open           = weasel_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
        .write          = weasel_proc_write,
};

static const struct file_operations list_proc_fops = {
        .owner          = THIS_MODULE,
        .open           = list_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
        .write          = weasel_proc_write,
};

static const struct file_operations pwd_proc_fops = {
        .owner          = THIS_MODULE,
        .open           = pwd_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
        .write          = weasel_proc_write,
};

static struct proc_dir_entry *dir;

static int pwd_proc_show(struct seq_file *m, void *v)
{
        unsigned int length = ((unsigned int) (-1)) >> d_hash_shift;
        int i;
        struct dentry *entry;
        struct hlist_bl_node *node;

        char temp[PATH_MAX];

        for(i = 0; i < length; ++i){
                hlist_bl_for_each_entry(entry, node, dentry_hashtable + i, d_hash){
                        if(entry->d_inode == NULL)
                                seq_printf(m, "%s\n", dentry_path_raw(entry, temp, PATH_MAX));
                }
        }

        return 0;
}

static int pwd_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, pwd_proc_show, NULL);
}

static int list_proc_show(struct seq_file *m, void *v)
{
        unsigned int length = ((unsigned int) (-1)) >> d_hash_shift;
        int i;
        struct dentry *entry;
        struct hlist_bl_node *node;

        char temp[PATH_MAX];

        for(i = 0; i < length; ++i){
                hlist_bl_for_each_entry(entry, node, dentry_hashtable + i, d_hash){
                        seq_printf(m, "%s\n", dentry_path_raw(entry, temp, PATH_MAX));
                }
        }

        return 0;
}

static int list_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, list_proc_show, NULL);
}

static int weasel_proc_show(struct seq_file *m, void *v)
{
        seq_printf(m, "I'm a weasel !\n");
        return 0;
}

static int weasel_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, weasel_proc_show, NULL);
}

static ssize_t weasel_proc_write(struct file *file,
                const char __user *buffer, size_t count, loff_t *pos)
{
        return count;
}

static void show(void)
{
        unsigned int length = ((unsigned int) (-1)) >> d_hash_shift;

        struct dentry *entry;
        struct hlist_bl_node *node;
        unsigned long long nb_elem = 0, temp, max = 0;
        int i;

        pr_info("Hello, weasel weasel\n");
        pr_info("L'adresse de la table est %p, sa taille est %u\n", dentry_hashtable, length);

        for(i = 0; i < length; ++i){
                temp = 0;
                hlist_bl_for_each_entry(entry, node, dentry_hashtable + i, d_hash){
                        nb_elem++;
                        temp++;
                }

                if(temp > max)
                        max = temp;
        }

        pr_info("Il y a %llu dentry éléments dans la table, et la liste la plus longue contient %llu élements.\n", 
                nb_elem, max);
}

static int __init weasel_init(void)
{     
        show();

        dir = proc_mkdir("weasel", NULL);
        if(!dir)
                goto ERROR1;

        if (!proc_create("whoami", 0, dir, &weasel_proc_fops))
                goto ERROR2;

        if (!proc_create("list", 0, dir, &list_proc_fops))
                goto ERROR3;

        if (!proc_create("pwd", 0, dir, &pwd_proc_fops))
                goto ERROR4;

        return 0;

ERROR4:
        remove_proc_entry("list", dir);

ERROR3:
        remove_proc_entry("whoami", dir);

ERROR2:
        remove_proc_entry("weasel", NULL);

ERROR1:
        return -1;
}

static void __exit weasel_exit(void)
{
        pr_info("tututu tu tu tu tuuu tututu tu tu\n");
        remove_proc_entry("whoami", dir);
        remove_proc_entry("list", dir);
        remove_proc_entry("pwd", dir);
        remove_proc_entry("weasel", NULL);
}