#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/proc_fs.h>
#include<asm/uaccess.h>

static struct proc_dir_entry *mydir;
static struct proc_dir_entry *pfile;
static char msg[255];

static int myproc_read(char *page,char **start,off_t off,int count,int *eof,void *data)
{
	int len = strlen(msg);
	if(off >= len)
		return 0;
	if(count > len-off)
		count = len - off;
	memcpy(page + off,msg + off, count);
	return off +count;
}
int myproc_write(struct file *file,const char __user *buffer,unsigned long count,void *data )
{
	unsigned long count2 = count;
	if(count2 >= sizeof(msg))
		count2 = sizeof(msg)-1;
	
	if(copy_from_user(msg,buffer,count2))//从buffer中拿到数据复制到msg长度是count2
		return -EFAULT;
	msg[count2] = '\0';//因为是字符串加个结尾符
	return count;
}
static int __init myproc_init(void)
{
	mydir = proc_mkdir("mydir",NULL);
	if(!mydir)
	{
		printk(KERN_ERR "can't creat /proc/mydir\n");
		return -1;
	}
	pfile = create_proc_entry("pool",0666,mydir);//在/proc/mydir/生成
	if(!pfile)
	{
		remove_proc_entry("mydir",NULL);
		printk(KERN_ERR "Can't create /proc/mydir/pool \n");
		return -1;
	}
	pfile->read_proc = myproc_read;
	pfile->write_proc = myproc_write;
	
	return 0;
}
static void __exit myproc_exit(void)
{
	remove_proc_entry("pool",NULL);
	remove_proc_entry("mydir",NULL);
}
module_init(myproc_init);
module_exit(myproc_exit);
MODULE_LICENSE("GPL");



