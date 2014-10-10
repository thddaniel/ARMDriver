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
#include<linux/timer.h>

static struct class *key1_class;
static struct class_device *key1_class_dev;

 struct timer_list timer;
 unsigned char key_vals[4];
 	int cnt=0;

 volatile unsigned long *GPFCON = NULL;
 volatile unsigned long *GPFDAT = NULL;
 /*作为指令关键字，确保本条指令不会因编译器的优化而省略，且要求每次直接读值. 　
 　简单地说就是防止编译器对代码进行优化.*/

 static int key1_open(struct inode *inode,struct file *file)
 {
	*GPFCON &=~((0x3<<(0*2))|(0x3<<(1*2))|(0x3<<(2*2))|(0x3<<(4*2)));
 	return 0;
 }

 ssize_t key1_read(struct file *file, char __user *buf,size_t size, loff_t *ppos)
 {
 	//unsigned char key_vals[4];
	int regval;

	if (size != sizeof(key_vals))
		return -EINVAL; 

	regval = *GPFDAT;
	key_vals[0] = (regval & (1<<0)) ? 1 : 0;
	key_vals[1] = (regval & (1<<1)) ? 1 : 0; 
	key_vals[2] = (regval & (1<<2)) ? 1 : 0; 
	key_vals[3] = (regval & (1<<4)) ? 1 : 0; 
	
	//copy_to_user(buf,key_vals,sizeof(key_vals)); 
 	mod_timer(&timer, jiffies + 100);
 	return sizeof(key_vals);
 }
 void timer_function(unsigned long data)
 {
	if(!key_vals[0]||!key_vals[1]||!key_vals[2]||!key_vals[3])
		{
			printk("%04d key pressed: %d %d %d %d\n",cnt++,key_vals[0],key_vals[1],key_vals[2],key_vals[3]);
		}
	 
 }

 static struct file_operations key1_fops=
 {
 	.owner = THIS_MODULE,
	.open  = key1_open,
	.read  = key1_read, 
 };


 int major;
 static int key1_init(void)
 {
 	major=register_chrdev(0,"key1",&key1_fops);	 //注册驱动程序

	key1_class = class_create(THIS_MODULE, "key1");

	key1_class_dev = device_create(key1_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/buttons */

	GPFCON = (volatile unsigned long *)ioremap(0x56000050, 16);
	GPFDAT = GPFCON + 1;

	init_timer(&timer);
	timer.function = timer_function;

	return 0;
 }


 static void key1_exit(void)
 {
	unregister_chrdev(major,"key1");	//卸载

	device_unregister(key1_class_dev);
	class_destroy(key1_class);

	iounmap(GPFCON);
	del_timer(&timer);
	
 }


 module_init(key1_init);
 module_exit(key1_exit);

 MODULE_LICENSE("GPL");
