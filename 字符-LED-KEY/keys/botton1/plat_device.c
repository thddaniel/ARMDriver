#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <mach/regs-gpio.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/io.h>

/*平台资源的定义*/
static struct resource s3c_buttons_resource[] = {
	[0]={
		.start = S3C24XX_PA_GPIO,
		.end   = S3C24XX_PA_GPIO + S3C24XX_SZ_GPIO - 1,
		.flags = IORESOURCE_MEM,
	},
	
	[1]={
		.start = IRQ_EINT0,
		.end   = IRQ_EINT0,
		.flags = IORESOURCE_IRQ,
	},
	[2]={
		.start = IRQ_EINT1,
		.end   = IRQ_EINT1,
		.flags = IORESOURCE_IRQ,
	},
	[3]={
		.start = IRQ_EINT2,
		.end   = IRQ_EINT2,
		.flags = IORESOURCE_IRQ,
	},
	[4]={
		.start = IRQ_EINT4,
		.end   = IRQ_EINT4,
		.flags = IORESOURCE_IRQ,
	},
	[5]={
		.start = IRQ_EINT15,
		.end   = IRQ_EINT15,
		.flags = IORESOURCE_IRQ,
	},
	[6]={
		.start = IRQ_EINT19,
		.end   = IRQ_EINT19,
		.flags = IORESOURCE_IRQ,
	}
};

static struct platform_device *s3c_buttons;


static int __init platform_init(void)
{

    s3c_buttons = platform_device_alloc("mini2440-buttons",-1);

    platform_device_add_resources(s3c_buttons,&s3c_buttons_resource,7);

    /*平台设备的注册*/
    platform_device_add(s3c_buttons);


}

static void __exit platform_exit(void)
{
    platform_device_unregister(s3c_buttons);
}

module_init(platform_init);
module_exit(platform_exit);

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mini2440buttons");
