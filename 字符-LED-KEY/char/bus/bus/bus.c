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
������: strncmp ������ ��: ���Ƚ� ������ ��: int strncmp(char *str1, char *str2, int maxlen); ���
�˺������ܼ��Ƚ��ַ���str1��str2��ǰmaxlen���ַ���
���ǰmaxlen�ֽ���ȫ��ȣ�����ֵ��=0��
��ǰmaxlen�ֽڱȽϹ����У��������str1[n]��str2[n]���ȣ��򷵻أ�str1[n]-str2[n]���� 
*/
static void my_bus_release(struct device *dev)
{
	printk(KERN_DEBUG "my bus release\n");
}



	
struct device my_bus = {
	
	.init_name   = "my_bus0",//��������� ���豸������ʲô��ϵ��
	.release  = my_bus_release
};


struct bus_type my_bus_type = {
	.name = "my_bus",
	.match = my_match,
};

EXPORT_SYMBOL(my_bus);
EXPORT_SYMBOL(my_bus_type);//�������



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
        
        /*ע������*/
	ret = bus_register(&my_bus_type);
	if (ret)
		return ret;
		
	/*���������ļ�*/	
	if (bus_create_file(&my_bus_type, &bus_attr_version))
		printk(KERN_NOTICE "Fail to create version attribute!\n");
	
	/*ע�������豸*/
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

