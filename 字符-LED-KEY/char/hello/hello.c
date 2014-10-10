#include <linux/module.h>
#include <linux/init.h>
 
MODULE_LICENSE("GPL");

static int __init hello_init(void)
{
	printk("<1>\n	Hello!\n");
	printk("<1>\n This is first my driver program.\n\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk("<1>\n	Exit!\n");
	printk("<1>\n Goodbye!\n\n");
}

module_init(hello_init);
module_exit(hello_exit);
