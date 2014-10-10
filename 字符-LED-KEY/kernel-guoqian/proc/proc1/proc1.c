#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/proc_fs.h>
#define procfs_name "proctest"

struct proc_dir_entry *Our_Proc_File;
int procfile_read(char *buffer,char **buffer_location,off_t offset,int buffer_length,int *eof,void *data)
{
	int ret;
	ret = sprintf(buffer,"HelloWorld!\n");
	return ret;
}
int proc_init(void)
{
	Our_Proc_File=create_proc_entry(procfs_name,0644,NULL);
	if(Our_Proc_File==NULL)
	{
		remove_proc_entry(procfs_name,NULL);
		printk(KERN_ALERT "Error:Could not initialize /proc/%s\n",procfs_name);
		return -ENOMEM;
	}
	Our_Proc_File->read_proc = procfile_read;
	Our_Proc_File->owner = THIS_MODULE;
	Our_Proc_File->mode = S_IFREG | S_IRUGO;
	Our_Proc_File->uid = 0;
	Our_Proc_File->gid = 0;
	Our_Proc_File->size = 37;
	printk("/proc/%s created\n",procfs_name);
	return 0;
}
void proc_exit(void)
{
	remove_proc_entry(procfs_name,NULL);
	printk(KERN_INFO "/proc/%s removed\n",procfs_name);
}
module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");


