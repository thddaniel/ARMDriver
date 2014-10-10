#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/platform_device.h>


static int my_probe(struct platform_device *dev)
{
	printk("Driver found device which my driver can handle!\n");
	return 0;
}

static int my_remove(struct platform_device *dev)
{
	printk("Driver found device unpluged!\n");
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
	/*×¢²áÆ½Ì¨Çý¶¯*/
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

