#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>


static char *Version = "$Revision: 1.9 $";


static int my_match(struct device *dev, struct device_driver *driver)
{
   
    
   //printk("dev->init_name:%s\tdriver->name:%s\n", dev->init_name, driver->name);
   //return !strncmp( dev->init_name , driver->name, strlen(driver->name));
    return !strncmp( dev_name(dev) ,  driver->name, strlen(driver->name));
}
/*
函数名: strncmp 　　功 能: 串比较 　　用 法: int strncmp(char *str1, char *str2, int maxlen); 　�
此函数功能即比较字符串str1和str2的前maxlen个字符。
如果前maxlen字节完全相等，返回值就=0；
在前maxlen字节比较过程中，如果出现str1[n]与str2[n]不等，则返回（str1[n]-str2[n]）。 
*/
static void my_bus_release(struct device *dev)
{
	printk(KERN_DEBUG "my bus release\n");
}



	
struct device my_bus = {
	
	.init_name   = "my_bus0",//这里的名字 和设备驱动有什么关系？
	.release  = my_bus_release
};


struct bus_type my_bus_type = {
	.name = "my_bus",
	.match = my_match,
};

EXPORT_SYMBOL(my_bus);
EXPORT_SYMBOL(my_bus_type);//符号输出



/*
 * Export a simple attribute.
 */
static ssize_t show_bus_version(struct bus_type *bus, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", Version);
}

static BUS_ATTR(version, S_IRUGO, show_bus_version, NULL);


static int __init my_bus_init(void)
{
	int ret;
        
        /*注册总线*/
	ret = bus_register(&my_bus_type);
	if (ret)
		return ret;
		
	/*创建属性文件*/	
	if (bus_create_file(&my_bus_type, &bus_attr_version))
		printk(KERN_NOTICE "Fail to create version attribute!\n");
	
	/*注册总线设备*/
	ret = device_register(&my_bus);
	if (ret)
		printk(KERN_NOTICE "Fail to register device:my_bus!\n");
		
	return ret;
}

static void my_bus_exit(void)
{
	device_unregister(&my_bus);
	bus_unregister(&my_bus_type);
}

module_init(my_bus_init);
module_exit(my_bus_exit);

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("Dual BSD/GPL");

