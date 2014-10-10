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
 //static struct class_device	*buttons_class_dev;


 #define DEVICE_NAME      "buttons"
 #define DEV_COUNT       1
 #define BUTTON_MAJOR    0
 #define BUTTON_MINOR    0
  
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
  
  dev_t dev_num;
  static struct cdev * buttons_cdev_p;  //cdev�ṹ��ָ��
  
    
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
 
     memset((void *)press_cnt,0,sizeof(press_cnt));	 //��������0
 
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


    /*  ��ʼ����ע��cdev*/
 static void buttons_cdev_setup(void)
 {
     int err;
     cdev_init(buttons_cdev_p,&buttons_fops);
     buttons_cdev_p->owner = THIS_MODULE;
     buttons_cdev_p->ops = &buttons_fops;
     err = cdev_add(buttons_cdev_p,dev_num,1);
     if(IS_ERR(&err))
         printk(KERN_NOTICE "Error %d adding buttons",err);
 }


 static int buttons_init(void)
 {
     int ret;
 
     /*ע���ַ��豸��������
         *����Ϊ���豸�ţ��豸���֣�file_operations�ṹ��
         *���������豸�žͺ;����file_operations�ṹ����ϵ�����ˣ�
         *�������豸��ΪBUTTON_MAJOR���豸�ļ�ʱ���ͻ����
         *tq2440_buttons_fops�е���س�Ա������BUTTON_MAJOR������Ϊ0��
         *��ʾ���ں��Զ��������豸��
         */
  
        if(BUTTON_MAJOR)   //�ֶ������豸��
            {  
              dev_num=MKDEV(BUTTON_MAJOR, BUTTON_MINOR);
                ret=register_chrdev_region(dev_num,DEV_COUNT,DEVICE_NAME);
             }
     else  
	 {          //��̬�����豸��
         ret = alloc_chrdev_region(&dev_num,BUTTON_MINOR,DEV_COUNT,DEVICE_NAME);        
     }
     if(ret<0)    
	  {
                 printk(DEVICE_NAME " can't register major number \n");
                 return ret;
      }
     /*��̬����cdev�ṹ����ڴ�*/
     buttons_cdev_p = kmalloc(sizeof(struct cdev),GFP_KERNEL);
     if(!buttons_cdev_p)   //����ʧ��
         {
             ret = -ENOMEM;
             goto fial_malloc;
         }
     memset(buttons_cdev_p,0,sizeof(struct cdev));
      buttons_cdev_setup();
      //ע��һ���࣬ʹmdev������"/dev/"Ŀ¼����
      //�����豸�ڵ�
      buttons_class = class_create(THIS_MODULE,DEVICE_NAME);
      if(IS_ERR(buttons_class))
          {
          printk("Error:Failed to creat button_class \n");
          return -1;
         }
      //����һ���豸�ڵ㣬�ڵ���ΪDEVICE_NAME
      
      device_create(buttons_class,NULL,dev_num,NULL,DEVICE_NAME);
      printk(DEVICE_NAME " initialized \n");
      return 0;
      fial_malloc:unregister_chrdev_region(dev_num, 1);
  }   
  
  
    /*ж����������*/
  static void buttons_exit(void)
  {
      cdev_del(buttons_cdev_p);//ע��cdev
      kfree(buttons_cdev_p);//�ͷ��豸�ṹ���ڴ�
      unregister_chrdev_region(dev_num,DEV_COUNT);//�ͷ��豸�� 
  }

            
  /*�������ƶ���������ĳ�ʼ��������ж�غ���*/
  module_init(buttons_init);
  module_exit(buttons_exit);

  MODULE_LICENSE("GPL");
