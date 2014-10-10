#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/device.h>

#include "memdev.h"

static struct class *myclass;
static struct class_device *class_dev;

static mem_major = MEMDEV_MAJOR;

module_param(mem_major, int, S_IRUGO);

struct mem_dev *mem_devp;	  /*设备结构体指针*/

struct cdev cdev;



/*文件打开函数*/
int mem_open(struct inode *inode, struct file *filp)
{
	struct mem_dev *dev;

    /*获取次设备号*/
	int num = MINOR(inode->i_rdev);

	if (num >= MEMDEV_NR_DEVS)
		return -ENODEV;

	dev = &mem_devp[num];

	/*将设备描述结构指针赋值给文件私有数据指针*/
	filp->private_data = dev;

	return 0;
	/*container_of实现了根据一个结构体变量中的一个域成员变量的指针来获取指向整个结构体变量的指针的功能。*/
}

/*文件释放函数*/
int mem_release(struct inode *inode, struct file *filp)
{
	return 0;
}



/*读函数*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct mem_dev *dev = filp->private_data;	 /*获得设备结构体指针*/

    /*判断读位置是否有效*/	 
		if (p >= MEMDEV_SIZE)
		return 0;
	if (count > MEMDEV_SIZE - p)
		count = MEMDEV_SIZE - p;

	/*读数据到用户空间*/
	if (copy_to_user(buf, (void*) (dev->data+p), count))
	{
		ret = -EFAULT;
	}
	else
	{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "read %d bytes(s) from %d\n", count, p);
	}

	return ret;
}



/*写函数*/
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct mem_dev *dev = filp->private_data;

	if (p >=MEMDEV_SIZE)
		return 0;
	if (count > MEMDEV_SIZE - p)
		count = MEMDEV_SIZE -p;

	if (copy_from_user(dev->data + p, buf, count))
		ret = -EFAULT;
	else
	{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "writen %d bytes(s) from %d\n", count, p);
	}

	return ret;
}


/*seek文件定位函数*/
static loff_t mem_llseek(struct file *filp, loff_t offset, int whence)
{
	loff_t newpos;

	switch(whence)
	{
		case 0:	 /*SEEK_SET */
			newpos = offset;
			break;

		case 1:	 /*SEEK_CUR */
			newpos = filp->f_pos + offset;
			break;

		case 2:	 /*SEEK_END */
			newpos = MEMDEV_SIZE -1 +offset;
			break;

		default:   /*can't happen */
			return -EINVAL;
	}
	if ((newpos <0)|| (newpos >MEMDEV_SIZE))
		return -EINVAL;

	filp->f_pos = newpos;
	return newpos;
}



/*文件操作结构体*/
static const struct file_operations mem_fops =
{
	.owner = THIS_MODULE,
	.llseek = mem_llseek,
	.read = mem_read,
	.write = mem_write,
	.open = mem_open,
	.release = mem_release,	
};


static int memdev_init(void)
{
	int result;
	int i;

	dev_t devno = MKDEV(mem_major, 0);

	/*静态申请设备号
	   register_chrdev_region(dev_t first,unsigned int count,char *name)
	 First :要分配的设备编号范围的初始值(次设备号常设为0);
	 Count:表示从起始设备号开始连续的设备号数目，需要注意的是 count 不能过大，不然有可能溢出到下一个主设备号上；
	  Name:编号相关联的设备名称. (/proc/devices); 
	*/
	if(mem_major)
		result = register_chrdev_region(devno, 2, "memdev");/*次设备0~1 对应 mem_fops  2~255 都不对应*/
	else  
		/*动态分配设备号
		参数 dev ，在系统调用成功后，会把得到的设备号方到这个参数中；
		参数 firstminor 是请求的第一个次设备号，一般为 0 ；
		参数 count	表示一个范围值；
		参数 name 表示设备名。
		*/
	{
		result = alloc_chrdev_region(&devno, 0, 2, "memdev");
		mem_major = MAJOR(devno);
	}

	if (result < 0)
		return result;

	cdev_init(&cdev, &mem_fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &mem_fops;

	/*注册字符设备*/
	cdev_add(&cdev, MKDEV(mem_major, 0), MEMDEV_NR_DEVS);






	/*为设备描述结构分配内存*/  
	mem_devp = kmalloc(MEMDEV_NR_DEVS * sizeof(struct mem_dev), GFP_KERNEL);
	if (!mem_devp)	 /*申请失败*/
	{
		result = -ENOMEM;
		goto fail_malloc;
	}

	memset(mem_devp, 0, sizeof(struct mem_dev));

	/*为设备分配内存*/
	for (i=0; i< MEMDEV_NR_DEVS; i++)
	{
		mem_devp[i].size = MEMDEV_SIZE;
		mem_devp[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
		memset(mem_devp[i].data, 0, MEMDEV_SIZE);
	}






	/*自动创建设备文件*/
	    myclass = class_create(THIS_MODULE, "test_char");/* file_operations中.owner = THIS_MODULE,在 sys 下创建类目录 /sys/class/test_char；(类的名字)*/
		class_dev = device_create(myclass,NULL, MKDEV(mem_major, 0), NULL, "memdev0");
		/* 
		device_create 在 sys 下创建设备目录 /sys/device/memdev0，如果它所属的类不为空，
		创建 /sys/class/test_char/memdev0 到 /sys/device/memdev0 的链接；
		*/
	return 0;

	fail_malloc:
	unregister_chrdev_region(devno, 1);

	return result;
}

static void memdev_exit(void)
{
	cdev_del(&cdev);
	kfree(mem_devp);
	unregister_chrdev_region(MKDEV(mem_major, 0), 2);
	
	device_unregister(class_dev);
	class_destroy(myclass);
}

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("GPL");

module_init(memdev_init);
module_exit(memdev_exit);
