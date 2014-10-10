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

struct mem_dev *mem_devp;	  /*�豸�ṹ��ָ��*/

struct cdev cdev;



/*�ļ��򿪺���*/
int mem_open(struct inode *inode, struct file *filp)
{
	struct mem_dev *dev;

    /*��ȡ���豸��*/
	int num = MINOR(inode->i_rdev);

	if (num >= MEMDEV_NR_DEVS)
		return -ENODEV;

	dev = &mem_devp[num];

	/*���豸�����ṹָ�븳ֵ���ļ�˽������ָ��*/
	filp->private_data = dev;

	return 0;
	/*container_ofʵ���˸���һ���ṹ������е�һ�����Ա������ָ������ȡָ�������ṹ�������ָ��Ĺ��ܡ�*/
}

/*�ļ��ͷź���*/
int mem_release(struct inode *inode, struct file *filp)
{
	return 0;
}



/*������*/
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct mem_dev *dev = filp->private_data;	 /*����豸�ṹ��ָ��*/

    /*�ж϶�λ���Ƿ���Ч*/	 
		if (p >= MEMDEV_SIZE)
		return 0;
	if (count > MEMDEV_SIZE - p)
		count = MEMDEV_SIZE - p;

	/*�����ݵ��û��ռ�*/
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



/*д����*/
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


/*seek�ļ���λ����*/
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



/*�ļ������ṹ��*/
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

	/*��̬�����豸��
	   register_chrdev_region(dev_t first,unsigned int count,char *name)
	 First :Ҫ������豸��ŷ�Χ�ĳ�ʼֵ(���豸�ų���Ϊ0);
	 Count:��ʾ����ʼ�豸�ſ�ʼ�������豸����Ŀ����Ҫע����� count ���ܹ��󣬲�Ȼ�п����������һ�����豸���ϣ�
	  Name:�����������豸����. (/proc/devices); 
	*/
	if(mem_major)
		result = register_chrdev_region(devno, 2, "memdev");/*���豸0~1 ��Ӧ mem_fops  2~255 ������Ӧ*/
	else  
		/*��̬�����豸��
		���� dev ����ϵͳ���óɹ��󣬻�ѵõ����豸�ŷ�����������У�
		���� firstminor ������ĵ�һ�����豸�ţ�һ��Ϊ 0 ��
		���� count	��ʾһ����Χֵ��
		���� name ��ʾ�豸����
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

	/*ע���ַ��豸*/
	cdev_add(&cdev, MKDEV(mem_major, 0), MEMDEV_NR_DEVS);






	/*Ϊ�豸�����ṹ�����ڴ�*/  
	mem_devp = kmalloc(MEMDEV_NR_DEVS * sizeof(struct mem_dev), GFP_KERNEL);
	if (!mem_devp)	 /*����ʧ��*/
	{
		result = -ENOMEM;
		goto fail_malloc;
	}

	memset(mem_devp, 0, sizeof(struct mem_dev));

	/*Ϊ�豸�����ڴ�*/
	for (i=0; i< MEMDEV_NR_DEVS; i++)
	{
		mem_devp[i].size = MEMDEV_SIZE;
		mem_devp[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
		memset(mem_devp[i].data, 0, MEMDEV_SIZE);
	}






	/*�Զ������豸�ļ�*/
	    myclass = class_create(THIS_MODULE, "test_char");/* file_operations��.owner = THIS_MODULE,�� sys �´�����Ŀ¼ /sys/class/test_char��(�������)*/
		class_dev = device_create(myclass,NULL, MKDEV(mem_major, 0), NULL, "memdev0");
		/* 
		device_create �� sys �´����豸Ŀ¼ /sys/device/memdev0��������������಻Ϊ�գ�
		���� /sys/class/test_char/memdev0 �� /sys/device/memdev0 �����ӣ�
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
