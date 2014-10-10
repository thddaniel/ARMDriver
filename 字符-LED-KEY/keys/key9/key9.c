#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/proc_fs.h> 
#include <linux/sysctl.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/poll.h>

   
struct pin_desc{
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};


struct pin_desc pins_desc[4] = {
	{IRQ_EINT1, "K1", S3C2410_GPF1, KEY_L},
	{IRQ_EINT4, "K2", S3C2410_GPF4, KEY_S},
	{IRQ_EINT2, "K3", S3C2410_GPF2, KEY_ENTER},
	{IRQ_EINT0, "K4", S3C2410_GPF0, KEY_LEFTSHIFT},
};


static struct input_dev *buttons_dev;
static struct pin_desc *irq_pd;
static struct timer_list buttons_timer;



/*
  * ȷ������ֵ
  */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	/* 10ms��������ʱ�� */
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies+HZ/100);
	return IRQ_RETVAL(IRQ_HANDLED);
}

 
static void buttons_timer_function(unsigned long data)
{
	struct pin_desc * pindesc = irq_pd;
	unsigned int pinval;

	if (!pindesc)
		return;
	
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if (pinval)
	{
		/* �ɿ� : ���һ������: 0-�ɿ�, 1-���� */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
		input_sync(buttons_dev);
	}
	else
	{
		/* ���� */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);
	}    
}

static int key2_init(void)
{
	int i;
	
	/* 1. ����һ��input_dev�ṹ�� */
	buttons_dev = input_allocate_device();

	/* 2. ���� */
	/* 2.1 �ܲ��������¼� */
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);

	/* 2.2 �ܲ���������������Щ�¼�: L,S,ENTER,LEFTSHIT */
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);

	/* 3. ע�� */
	input_register_device(buttons_dev);

	/* 4. Ӳ����صĲ��� */
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);

	for (i = 0; i < 4; i++)
	{
		request_irq(pins_desc[i].irq, buttons_irq, IRQ_TYPE_EDGE_FALLING, pins_desc[i].name, &pins_desc[i]);
	}

 	return 0;
}

static void key2_exit(void)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		free_irq(pins_desc[i].irq, &pins_desc[i]);
	}

	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev);	

	return 0;
}


module_init(key2_init);

module_exit(key2_exit);

MODULE_LICENSE("GPL");


