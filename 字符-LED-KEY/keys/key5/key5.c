#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/interrupt.h> 
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/poll.h>

 static struct class *key2_class;
 static struct class_device	*key2_class_dev;


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* �ж��¼���־, �жϷ����������1��third_drv_read������0 */
static volatile int ev_press = 0;


struct pin_desc
{
	unsigned int pin;
	unsigned int key_val;
};


/* ��ֵ: ����ʱ, 0x01, 0x02, 0x03, 0x04 */
/* ��ֵ: �ɿ�ʱ, 0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

/*
 * K1,K2,K3,K4��ӦGPF1��GPF4��GPF2��GPF0
 */

struct pin_desc pins_desc[4] = {
	{S3C2410_GPF1, 0x01},
	{S3C2410_GPF4, 0x02},
	{S3C2410_GPF2, 0x03},
	{S3C2410_GPF0, 0x04},
};


/*
  * ȷ������ֵ
  */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if (pinval)
	{
		/* �ɿ� */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* ���� */
		key_val = pindesc->key_val;
	}

    ev_press = 1;                  /* ��ʾ�жϷ����� */
    wake_up_interruptible(&button_waitq);   /* �������ߵĽ��� */
		
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int key2_open(struct inode *inode, struct file *file)
{
	/* GPF1��GPF4��GPF2��GPF0Ϊ�ж����� */	  
	  	
	  request_irq(IRQ_EINT1, buttons_irq, IRQ_TYPE_EDGE_FALLING, "K1", &pins_desc[0]);
	  request_irq(IRQ_EINT4, buttons_irq, IRQ_TYPE_EDGE_FALLING, "K2", &pins_desc[1]);
	  request_irq(IRQ_EINT2, buttons_irq, IRQ_TYPE_EDGE_FALLING, "K3", &pins_desc[2]);
	  request_irq(IRQ_EINT0, buttons_irq, IRQ_TYPE_EDGE_FALLING, "K4", &pins_desc[3]);
	  	
	  return 0;
}

ssize_t key2_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	/* ���û�а�������, ���� */
	wait_event_interruptible(button_waitq, ev_press);

	/* ����а�������, ���ؼ�ֵ */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}


int key2_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT1, &pins_desc[0]);
	free_irq(IRQ_EINT4, &pins_desc[1]);
	free_irq(IRQ_EINT2, &pins_desc[2]);
	free_irq(IRQ_EINT0, &pins_desc[3]);
	return 0;
}


static unsigned key2_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // ������������

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static struct file_operations key2_fops = {
    .owner   =  THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open    =  key2_open,     
	.read	 =	key2_read,	   
	.release =  key2_close,
	.poll    =  key2_poll,	   
};




int major;
static int key2_init(void)
{
	major = register_chrdev(0, "key2", &key2_fops);

	key2_class = class_create(THIS_MODULE, "key2");
	key2_class_dev = device_create(key2_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/buttons */

 	return 0;
}

static void key2_exit(void)
{
	unregister_chrdev(major, "key2");

	device_unregister(key2_class_dev);
	class_destroy(key2_class);

	return 0;
}


module_init(key2_init);

module_exit(key2_exit);

MODULE_LICENSE("GPL");


