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

#include "ioctl.h"
static struct class *myclass;
static int mem_major = MEMDEV_MAJOR;

module_param(mem_major, int, S_IRUGO);

struct mem_dev *mem_devp; /*�豸�ṹ��ָ��*/

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
}

/*�ļ��ͷź���*/
int mem_release(struct inode *inode, struct file *filp)
{
  return 0;
}


int memdev_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int ret = 0;
	int ioarg = 0;

	if (_IOC_TYPE(cmd) != MEMDEV_IOC_MAGIC)
		return -EINVAL;
	if (_IOC_NR(cmd) > MEMDEV_IOC_MAXNR)
		return -EINVAL;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	switch(cmd)
	{
		case MEMDEV_IOCPRINT:
			printk("<--- CMD MEMDEV_IOCPRINT Done--->\n\n");
			break;


	case MEMDEV_IOCSETDATA:
			ret = __get_user(ioarg, (int *)arg);
			printk("<--- In Kernel MEMDEV_IOCSETDATA ioarg = %d --->\n\n", ioarg);
			break;


		case MEMDEV_IOCGETDATA:
			ioarg = 1101;
			ret = __put_user(ioarg, (int *)arg);
			break;

		default:
			return -EINVAL;
	}
	return ret;
}
 
/*�ļ������ṹ��*/
static const struct file_operations mem_fops =
{
  .owner = THIS_MODULE,
  .open = mem_open,
  .release = mem_release,
  .ioctl = memdev_ioctl,
};

/*�豸����ģ����غ���*/
static int memdev_init(void)
{
  int result;
  int i;

  dev_t devno = MKDEV(mem_major, 0);

  /* ��̬�����豸��*/
  if (mem_major)
    result = register_chrdev_region(devno, 2, "memdev");
  else  /* ��̬�����豸�� */
  {
    result = alloc_chrdev_region(&devno, 0, 2, "memdev");
    mem_major = MAJOR(devno);
  }  
  
  if (result < 0)
    return result;

  /*��ʼ��cdev�ṹ*/
  cdev_init(&cdev, &mem_fops);
  cdev.owner = THIS_MODULE;
  cdev.ops = &mem_fops;
  
  /* ע���ַ��豸 */
  cdev_add(&cdev, MKDEV(mem_major, 0), MEMDEV_NR_DEVS);
   
  /* Ϊ�豸�����ṹ�����ڴ�*/
  mem_devp = kmalloc(MEMDEV_NR_DEVS * sizeof(struct mem_dev), GFP_KERNEL);
  if (!mem_devp)    /*����ʧ��*/
  {
    result =  - ENOMEM;
    goto fail_malloc;
  }
  memset(mem_devp, 0, sizeof(struct mem_dev));
  
  /*Ϊ�豸�����ڴ�*/
  for (i=0; i < MEMDEV_NR_DEVS; i++) 
  {
        mem_devp[i].size = MEMDEV_SIZE;
        mem_devp[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
        memset(mem_devp[i].data, 0, MEMDEV_SIZE);
  }
    /*�Զ������豸�ļ�*/
	    myclass = class_create(THIS_MODULE, "test_char");
		device_create(myclass,NULL, MKDEV(mem_major, 0), NULL, "memdev0");
  return 0;

  fail_malloc: 
  unregister_chrdev_region(devno, 1);
  
  return result;
}

/*ģ��ж�غ���*/
static void memdev_exit(void)
{
  cdev_del(&cdev);   /*ע���豸*/
  kfree(mem_devp);     /*�ͷ��豸�ṹ���ڴ�*/	
  unregister_chrdev_region(MKDEV(mem_major, 0), 2); /*�ͷ��豸��*/
  class_destroy(myclass);
}

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("GPL");

module_init(memdev_init);
module_exit(memdev_exit);