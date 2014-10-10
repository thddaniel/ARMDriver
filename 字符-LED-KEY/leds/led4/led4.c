#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h> 	
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/miscdevice.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/unistd.h>

#define DEVICE_NAME "leds"

static unsigned long led_table[] =
{
	S3C2410_GPB5,
	S3C2410_GPB6,
	S3C2410_GPB7,
	S3C2410_GPB8,
};

static unsigned int led_cfg_table[] =
{
	S3C2410_GPB5_OUTP,
	S3C2410_GPB6_OUTP,
	S3C2410_GPB7_OUTP,
	S3C2410_GPB8_OUTP,
};

static int sbc2440_leds_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case 0:
		case 1:
			if(arg >4)
			{
				return -EINVAL;
			}
		s3c2410_gpio_setpin(led_table[arg],cmd);
		return 0;
	}
	
}


 static struct file_operations dev_fops = 
 {
 	.owner   =  THIS_MODULE, 
	.ioctl	 = 	sbc2440_leds_ioctl,
 };

 static struct miscdevice misc = 
 {
 	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEVICE_NAME,
	.fops  = &dev_fops,
 };


 
 static int __init dev_init(void)
 {
	int ret;
	int i;

	for (i=0;i<4;i++)
	{
		s3c2410_gpio_cfgpin(led_table[i],led_cfg_table[i]);
		s3c2410_gpio_setpin(led_table[i],0);
	}

	ret = misc_register(&misc);

	printk (DEVICE_NAME"\tinitialized\n");

	return 0;
 }


 static void __exit dev_exit(void)
 {
	 misc_deregister(&misc);
 }


 module_init(dev_init);
 module_exit(dev_exit);

 MODULE_LICENSE("GPL");