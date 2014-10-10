  #include <linux/module.h> 
  #include <linux/kernel.h> 
  #include <linux/fs.h> 
  #include <linux/init.h> 
  #include <linux/delay.h> 
  #include <linux/poll.h> 
  #include <linux/irq.h>    
  #include <linux/io.h>
  #include <linux/interrupt.h> 
  #include <linux/device.h>
  #include <linux/platform_device.h> 
  #include <linux/cdev.h> 
  #include <linux/miscdevice.h>
  #include <plat/regs-adc.h>
  #include <mach/regs-clock.h>
  #include <mach/regs-gpio.h> 
  #include <mach/hardware.h> 
  #include <asm/uaccess.h>
  #include <asm/irq.h>
  #include <asm/io.h> 
  

 static struct class *buttons_class;
 static struct class_device	*buttons_class_dev;
 
  
  #define MIN(A,B)        (A)>(B)?(B):(A)
  /*�ȴ�����:
  *��û�а���������ʱ������н��̵���tq2440_buttons_read����
  *��������*/
  
  static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
  /*�ж��¼���־,�жϷ����������1��tq2440_buttons_read������0 */
  
  static volatile int ev_press = 0;
  
  /* ���������µĴ���(׼ȷ��˵���Ƿ����жϵĴ���) */
  static volatile int  press_cnt[] = {0,0,0,0};
  
   
  struct button_irqs_desc      
  {
  int irq;   //�жϺ�
  unsigned long flags; //�жϱ�־�����������жϵĴ�����ʽ 
  char *name;        //�ж�����
  };
  
  /*����ָ���������õ��ⲿ�ж����ż��жϴ�����ʽ������*/
  static struct button_irqs_desc button_irqs[] =
 {
  {IRQ_EINT0,IRQ_TYPE_EDGE_FALLING,"KEY1"},  //K1
  {IRQ_EINT1,IRQ_TYPE_EDGE_FALLING,"KEY2"},  //K2
  {IRQ_EINT2,IRQ_TYPE_EDGE_FALLING,"KEY3"},  //K3
  {IRQ_EINT4,IRQ_TYPE_EDGE_FALLING,"KEY4"},  //K4
 };
  
     
  static irqreturn_t buttons_interrupt(int irq,void *dev_id)
  {
       volatile int *press_cnt = (volatile int *)dev_id;
  
      *press_cnt = *press_cnt + 1;     /*������������1  */
      ev_press = 1;                    /*��ʾ�жϷ�����*/
      wake_up_interruptible(&button_waitq);  /*�������ߵĽ���*/
  
      printk(" IRQ:%d\n",irq);
      
      return IRQ_RETVAL(IRQ_HANDLED);
  }
  
  /*Ӧ�ó���ִ��open("/dev/buttons",...)ϵͳ����ʱ��tq2440_buttons_open������
   *�����ã�������ע��4���������жϴ������*/
  static int buttons_open(struct inode *inode,struct file *file)
  {
     int i;
      int err;
  
      for(i =0;i<sizeof(button_irqs)/sizeof(button_irqs[0]);i++) 
	   {
         //ע���жϴ�����
         err = request_irq(button_irqs[i].irq,buttons_interrupt,button_irqs[i].flags,
                        button_irqs[i].name,(void *)&press_cnt[i]);
          if(err)    
              break;
       }
       if(err)
	   {
            //��������ͷ��Ѿ�ע����ж�
          i--;
        for(;i>=0;i--)
           free_irq(button_irqs[i].irq,(void *)&press_cnt[i]);
         return -EBUSY;
       }
       return 0;
 }
 
 /*Ӧ�ó�����豸�ļ�/dev/buttonsִ��close(...)ʱ��
  *�ͻ����tq2440_buttons_close����*/
  static int buttons_close(struct inode *inode,struct file *file)
 {
     int i;
 
     for(i=0;i<sizeof(button_irqs)/sizeof(button_irqs[0]);i++)  
	 {
         //�ͷ��Ѿ�ע����ж�
         free_irq(button_irqs[i].irq,(void *)&press_cnt[i]);
     }
     return 0;
 }
 
 /*Ӧ�ó�����豸�ļ�/dev/buttonsִ��read(...)ʱ��
  *�ͻ����tq2440_buttons_read����
  */
 static int buttons_read(struct file *filp,char __user *buff,size_t count,loff_t *offp)
 {
     unsigned long err;
     /*���ev_press����0������*/
     wait_event_interruptible(button_waitq,ev_press);
 
     /*ִ�е�����ʱev_press�϶�����1��������0 */
     ev_press = 0;
 
     /*������״̬���Ƹ��û�������0  */
     err = copy_to_user(buff,(const void *)press_cnt,MIN(sizeof(press_cnt),count));
 
     //memset((void *)press_cnt,0,sizeof(press_cnt));	 //��������0
 
     return err? -EFAULT:0;
 }
 

 /*����ṹ���ַ������豸����ĺ���
  *��Ӧ�ó�������豸�ļ�ʱ�����õ�open,read,write�Ⱥ���
  *���ջ��������ṹ�еĶ�Ӧ����
  */
 static struct file_operations buttons_fops = 
 {
     .owner    =  THIS_MODULE,/*����һ���ָ꣬�����ģ��ʱ�Զ�������__this_module����*/
     .open     =  buttons_open,
     .release  =  buttons_close,
     .read     =  buttons_read,
 };


int major;
static int buttons_init(void)
{
	major = register_chrdev(0, "buttons", &buttons_fops);

	buttons_class = class_create(THIS_MODULE, "buttons");
	buttons_class_dev = device_create(buttons_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

 	return 0;
}

 
static void buttons_exit(void)
{
	unregister_chrdev(major, "buttons");

	device_unregister(buttons_class_dev);
	class_destroy(buttons_class);

	 int i;

	return 0;
}

 
           
  /*�������ƶ���������ĳ�ʼ��������ж�غ���*/
  module_init(buttons_init);
  module_exit(buttons_exit);

  MODULE_LICENSE("GPL");
