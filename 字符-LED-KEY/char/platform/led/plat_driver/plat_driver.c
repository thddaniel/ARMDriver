#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/platform_device.h>



#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>



volatile unsigned long *GPBCON = NULL;
volatile unsigned long *GPBDAT = NULL;


static int my_probe(struct platform_device *dev)
{	
	//unsigned int a=5;
	
	GPBCON = (volatile unsigned long *)ioremap(0x56000010,16);
	GPBDAT = GPBCON + 1;//指针加1=地址加4
	*GPBCON &=~((1<<17)|(1<<15)|(1<<13)|(1<<11));
	*GPBCON |=((1<<16)|(1<<14)|(1<<12)|(1<<10));

	
	//while( a =! 0)
//{
	*GPBDAT &=~((1<<5)|(1<<6)|(1<<7)|(1<<8));
	ssleep(2);
	*GPBDAT |=((1<<5)|(1<<6)|(1<<7)|(1<<8));
	//a--;
//}	
	printk("led off \n");
	return 0;
}

static int my_remove(struct platform_device *dev)
{
	printk("Driver found device unpluged!\n");
	iounmap(GPBCON);
	return 0;
}

static struct platform_driver my_driver = 
{
	.probe	= my_probe,
	.remove = my_remove,
	.driver = 
	{
		.owner	= THIS_MODULE,
		.name	= "my_dev",
	}
};

static int __init my_driver_init(void)
{
	/*注册平台驱动*/
	return platform_driver_register(&my_driver);
}

static void my_driver_exit(void)
{
	platform_driver_unregister(&my_driver);
}

module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_AUTHOR("tang hao");
MODULE_LICENSE("Dual BSD/GPL");

