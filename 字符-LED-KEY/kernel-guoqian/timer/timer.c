#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/timer.h>
#include<asm/uaccess.h>

struct timer_list timer;

void timer_function(int para)
{
	printk("<0>Timer Expired and para is %d !!\n",para);
	
}
int timer_init()
{
	init_timer(&timer);
	timer.data = 5;
	timer.expires = jiffies + (20 *HZ);
	timer.function = timer_function;
	add_timer(&timer);
	return 0;

}
void timer_exit()
{
	del_timer(&timer);
}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");

























