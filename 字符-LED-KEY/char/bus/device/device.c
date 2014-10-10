#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>



extern struct device my_bus; 
extern struct bus_type my_bus_type;

/* Why need this ?*/
static void my_dev_release(struct device *dev)
{ 
	
}

struct device my_dev = {
	.bus = &my_bus_type,
	.init_name = "my_dev0",
	.parent = &my_bus,
	.release = my_dev_release,
};

/*
 * Export a simple attribute.
 */
static ssize_t mydev_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", "This is my device!");
}

static DEVICE_ATTR(dev, S_IRUGO, mydev_show, NULL);

static int __init my_device_init(void)
{
	int ret = 0;
        
        /* ��ʼ���豸 */
        /*Ϊʲô����ע���豸����struct device my_dev �����ʼ������*/
	//strncpy(my_dev.init_name, "my_dev0", BUS_ID_SIZE);
		
        
        /*ע���豸*/
	device_register(&my_dev);
		
	/*���������ļ�*/
	device_create_file(&my_dev, &dev_attr_dev);
	
	return ret;	

}

static void my_device_exit(void)
{
	device_unregister(&my_dev);
}

module_init(my_device_init);
module_exit(my_device_exit);

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("Dual BSD/GPL");

