#include <linux/module.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <mach/map.h>
#include <mach/regs-gpio.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/unistd.h>
#include <linux/device.h>


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

static volatile int ev_press = 0;


static int key_value;
static struct device     *buttons_dev;	/* platform device attached to */
static struct resource	*buttons_mem;
static struct resource   *buttons_irq;
static void __iomem	    *buttons_base;


static int button_irqs[6];

static irqreturn_t buttons_interrupt(int irq, void *dev_id)
{
	int i;
	for(i=0; i<6; i++){
		if(irq == button_irqs[i]){
			//printk("==>interrput number:%d\n",irq);  
			key_value = i;
			ev_press =1;
			wake_up_interruptible(&button_waitq);		
		}
	}
	
    return IRQ_RETVAL(IRQ_HANDLED);
	
}

static int s3c24xx_buttons_open(struct inode *inode, struct file *file)
{
	int i;
	int err = 0;
	/*ע���ж�*/
	for(i=0; i<6; i++){
		if (button_irqs[i] < 0) 
			continue;
						    
				/*�жϴ�����ʽ�������ش���*/
        err = request_irq(button_irqs[i],buttons_interrupt,IRQ_TYPE_EDGE_RISING,NULL,NULL);
		if(err)
		  break;
	}

	if (err) {
		i--;
		for (; i >= 0; i--) {
			if (button_irqs[i] < 0) {
				continue;
			}
			disable_irq(button_irqs[i]);
			free_irq(button_irqs[i], NULL);
		}
		return -EBUSY;
	}

    ev_press = 0;   
    return 0;
}

static int s3c24xx_buttons_close(struct inode *inode, struct file *file)
{
    int i;    
    for (i=0; i<6; i++) {
			if (button_irqs[i] < 0) {
				continue;
			}
			free_irq(button_irqs[i],NULL);
    }
    return 0;
}

static int s3c24xx_buttons_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    unsigned long err;
    if (!ev_press) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		else
			wait_event_interruptible(button_waitq, ev_press);
    }    
    ev_press = 0;
    err = copy_to_user(buff, &key_value, sizeof(key_value));
    return sizeof(key_value);
}

static unsigned int s3c24xx_buttons_poll( struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    poll_wait(file, &button_waitq, wait);
    if (ev_press){
        mask |= POLLIN | POLLRDNORM;
	}
    return mask;
}



static struct file_operations mini2440buttons_fops = {
    .owner   =   THIS_MODULE,
    .open    =   s3c24xx_buttons_open,
	.release =   s3c24xx_buttons_close,
    .read    =   s3c24xx_buttons_read,
	.poll    =   s3c24xx_buttons_poll,
};

static struct miscdevice mini2440_miscdev = {
	
    .minor = MISC_DYNAMIC_MINOR,
    .name ="buttons",
    .fops = &mini2440buttons_fops,
};

/* device interface */
static int mini2440_buttons_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct device *dev;
	int ret;
	int size;
	int i;
	
	printk("probe:%s\n", __func__);
	dev = &pdev->dev;
	buttons_dev = &pdev->dev;
	
	/*ƽ̨��Դ��ȡ*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(dev, "no memory resource specified\n");
		return -ENOENT;
	}
	
	size = (res->end - res->start) + 1;
	buttons_mem = request_mem_region(res->start, size, pdev->name);
	if (buttons_mem == NULL) {
		dev_err(dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto err_req;
	}
	
	buttons_base = ioremap(res->start, size);
	if (buttons_base == NULL) {
		dev_err(dev, "failed to ioremap() region\n");
		ret = -EINVAL;
		goto err_req;
	}
	printk(KERN_DEBUG"probe: mapped buttons_base=%p\n", buttons_base);

 /*get irq number*/
  for(i=0; i<6; i++){
	 buttons_irq = platform_get_resource(pdev,IORESOURCE_IRQ,i);
	 if(buttons_irq == NULL){
	   	dev_err(dev,"no irq resource specified\n");
		  ret = -ENOENT;
		  goto err_map;
	}
	button_irqs[i] = buttons_irq->start;
	//printk("button_irqs[%d]=%d\n",i,button_irqs[i]);  
	}
	ret = misc_register(&mini2440_miscdev);
		
	return 0;

 err_map:
	iounmap(buttons_base);

 err_req:
	release_resource(buttons_mem);
	kfree(buttons_mem);

	return ret;
}

static int mini2440_buttons_remove(struct platform_device *dev)
{
	release_resource(buttons_mem);
	kfree(buttons_mem);
	buttons_mem = NULL;

	iounmap(buttons_base);
	misc_deregister(&mini2440_miscdev);
	return 0;
}

/*ƽ̨��������*/
static struct platform_driver mini2440buttons_driver = {
	.probe		= mini2440_buttons_probe,
	.remove		= mini2440_buttons_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "mini2440-buttons",
	},
};

static char banner[] __initdata =
	 "Mini2440 Buttons Driver\n";

static int __init buttons_init(void)
{
	printk(banner);
	/*ƽ̨����ע��*/
	platform_driver_register(&mini2440buttons_driver);
	return 0;
}

static void __exit buttons_exit(void)
{
	platform_driver_unregister(&mini2440buttons_driver);
}

module_init(buttons_init);
module_exit(buttons_exit);

MODULE_AUTHOR("David Xie");
MODULE_DESCRIPTION("Mini2440 Buttons Driver");
MODULE_LICENSE("GPL");

