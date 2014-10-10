#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/platform_device.h> 
#include <linux/version.h> 
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/serial_core.h>
 


static struct platform_device *my_device;
 


static int my_device_init(void)
{
	int ret = 0;

	/* 分配结构图 */
	 my_device = platform_device_alloc("my_dev", -1);

	/*注册设备*/
     ret = platform_device_add(my_device);

	/*注册失败，释放相关内存*/
	if(ret)
		platform_device_put(my_device);

	return ret;
}


static void my_device_exit(void)
{
	platform_device_unregister(my_device);
}

module_init(my_device_init);
module_exit(my_device_exit);

MODULE_AUTHOR("tang hao");
MODULE_LICENSE("Dual BSD/GPL"); 

