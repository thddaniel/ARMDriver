#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/poll.h>

 
static struct class *firstdrv_class;
static struct class_device *firstdrv_class_dev;

 volatile unsigned long *GPBCON = NULL;
 volatile unsigned long *GPBDAT = NULL;

 /*作为指令关键字，确保本条指令不会因编译器的优化而省略，且要求每次直接读值. 　　简单地说就是防止编译器对代码进行优化.*/

 static int first_drv_open(struct inode *inode,struct file *file)
 {
 	printk("first_dev_open\n");
	*GPBCON &=~((1<<17)|(1<<15)|(1<<13)|(1<<11));
	*GPBCON |=((1<<16)|(1<<14)|(1<<12)|(1<<10));
 	return 0;
 }


 static ssize_t first_drv_write(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
 {
 	int val;

	printk("first_dev_write\n");

	copy_from_user(&val,buf,count);

	if (val == 1)
	{
		*GPBDAT &=~((1<<5)|(1<<6)|(1<<7)|(1<<8));
	}
	else
	{
		*GPBDAT |=((1<<5)|(1<<6)|(1<<7)|(1<<8));
	}
 	
 	return 0;	
 }

 static struct file_operations first_drv_fops=
 {
 	.owner = THIS_MODULE,
	.open  = first_drv_open,
	.write  = first_drv_write, 
 };


 int major;
 static int first_drv_init(void)
 {
 	major=register_chrdev(0,"first_drv",&first_drv_fops);	 //注册驱动程序

	firstdrv_class = class_create(THIS_MODULE, "first_drv");

	firstdrv_class_dev = device_create(firstdrv_class,NULL,MKDEV(major,0),NULL,"firstled");  // /dev/firstled

	GPBCON = (volatile unsigned long *)ioremap(0x56000010,16);
	GPBDAT = GPBCON + 1;

	return 0;
 }


 static void first_drv_exit(void)
 {
	unregister_chrdev(major,"first_drv");	//卸载

	  device_unregister(firstdrv_class_dev);
	  class_destroy(firstdrv_class);	

	iounmap(GPBCON);

	
 }


 module_init(first_drv_init);
 module_exit(first_drv_exit);

 MODULE_LICENSE("GPL");
