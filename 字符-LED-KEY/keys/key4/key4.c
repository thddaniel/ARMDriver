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
  /*等待队列:
  *当没有按键被按下时，如果有进程调用tq2440_buttons_read函数
  *它将休眠*/
  
  static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
  /*中断事件标志,中断服务程序将它置1，tq2440_buttons_read将它清0 */
  
  static volatile int ev_press = 0;
  
  /* 按键被按下的次数(准确地说，是发生中断的次数) */
  static volatile int  press_cnt[] = {0,0,0,0};
  
   
  struct button_irqs_desc      
  {
  int irq;   //中断号
  unsigned long flags; //中断标志，用来定义中断的触发方式 
  char *name;        //中断名称
  };
  
  /*用来指定按键所用的外部中断引脚及中断触发方式，名字*/
  static struct button_irqs_desc button_irqs[] =
 {
  {IRQ_EINT0,IRQ_TYPE_EDGE_FALLING,"KEY1"},  //K1
  {IRQ_EINT1,IRQ_TYPE_EDGE_FALLING,"KEY2"},  //K2
  {IRQ_EINT2,IRQ_TYPE_EDGE_FALLING,"KEY3"},  //K3
  {IRQ_EINT4,IRQ_TYPE_EDGE_FALLING,"KEY4"},  //K4
 };
  
  dev_t dev_num;
  static struct cdev * buttons_cdev_p;  //cdev结构体指针
  
    
  static irqreturn_t buttons_interrupt(int irq,void *dev_id)
  {
       volatile int *press_cnt = (volatile int *)dev_id;
  
      *press_cnt = *press_cnt + 1;     /*按键计数器加1  */
      ev_press = 1;                    /*表示中断发生了*/
      wake_up_interruptible(&button_waitq);  /*唤醒休眠的进程*/
  
      printk(" IRQ:%d\n",irq);
      
      return IRQ_RETVAL(IRQ_HANDLED);
  }
  
  /*应用程序执行open("/dev/buttons",...)系统调用时，tq2440_buttons_open函数将
   *被调用，它用来注册4个按键的中断处理程序*/
  static int buttons_open(struct inode *inode,struct file *file)
  {
     int i;
      int err;
  
      for(i =0;i<sizeof(button_irqs)/sizeof(button_irqs[0]);i++) 
	   {
         //注册中断处理函数
         err = request_irq(button_irqs[i].irq,buttons_interrupt,button_irqs[i].flags,
                        button_irqs[i].name,(void *)&press_cnt[i]);
          if(err)    
              break;
       }
       if(err)
	   {
            //如果出错，释放已经注册的中断
          i--;
        for(;i>=0;i--)
           free_irq(button_irqs[i].irq,(void *)&press_cnt[i]);
         return -EBUSY;
       }
       return 0;
 }
 
 /*应用程序对设备文件/dev/buttons执行close(...)时，
  *就会调用tq2440_buttons_close函数*/
  static int buttons_close(struct inode *inode,struct file *file)
 {
     int i;
 
     for(i=0;i<sizeof(button_irqs)/sizeof(button_irqs[0]);i++)  
	 {
         //释放已经注册的中断
         free_irq(button_irqs[i].irq,(void *)&press_cnt[i]);
     }
     return 0;
 }
 
 /*应用程序对设备文件/dev/buttons执行read(...)时，
  *就会调用tq2440_buttons_read函数
  */
  static int buttons_read(struct file *filp,char __user *buff,size_t count,loff_t *offp)
 {
     unsigned long err;
     /*如果ev_press等于0，休眠*/
     wait_event_interruptible(button_waitq,ev_press);
 
     /*执行到这里时ev_press肯定等于1，将它清0 */
     ev_press = 0;
 
     /*将按键状态复制给用户，并请0  */
     err = copy_to_user(buff,(const void *)press_cnt,MIN(sizeof(press_cnt),count));
 
     memset((void *)press_cnt,0,sizeof(press_cnt));	 //把数组清0
 
     return err? -EFAULT:0;
 }
 
 /*这个结构是字符驱动设备程序的核心
  *当应用程序操作设备文件时所调用的open,read,write等函数
  *最终会调用这个结构中的对应函数
  */
 
 static struct file_operations buttons_fops = 
 {
     .owner    =  THIS_MODULE,/*这是一个宏，指向编译模块时自动创建的__this_module变量*/
     .open     =  buttons_open,
     .release  =  buttons_close,
     .read     =  buttons_read,
 };


    /*  初始化并注册cdev*/
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
 
     /*注册字符设备驱动程序
         *参数为主设备号，设备名字，file_operations结构体
         *这样，主设备号就和具体的file_operations结构体联系起来了，
         *操作主设备号为BUTTON_MAJOR的设备文件时，就会调用
         *tq2440_buttons_fops中的相关成员函数，BUTTON_MAJOR可以设为0，
         *表示由内核自动分配主设备号
         */
  
        if(BUTTON_MAJOR)   //手动分配设备号
            {  
              dev_num=MKDEV(BUTTON_MAJOR, BUTTON_MINOR);
                ret=register_chrdev_region(dev_num,DEV_COUNT,DEVICE_NAME);
             }
     else  
	 {          //动态分配设备号
         ret = alloc_chrdev_region(&dev_num,BUTTON_MINOR,DEV_COUNT,DEVICE_NAME);        
     }
     if(ret<0)    
	  {
                 printk(DEVICE_NAME " can't register major number \n");
                 return ret;
      }
     /*动态申请cdev结构体的内存*/
     buttons_cdev_p = kmalloc(sizeof(struct cdev),GFP_KERNEL);
     if(!buttons_cdev_p)   //申请失败
         {
             ret = -ENOMEM;
             goto fial_malloc;
         }
     memset(buttons_cdev_p,0,sizeof(struct cdev));
      buttons_cdev_setup();
      //注册一个类，使mdev可以在"/dev/"目录下面
      //建立设备节点
      buttons_class = class_create(THIS_MODULE,DEVICE_NAME);
      if(IS_ERR(buttons_class))
          {
          printk("Error:Failed to creat button_class \n");
          return -1;
         }
      //创建一个设备节点，节点名为DEVICE_NAME
      
      device_create(buttons_class,NULL,dev_num,NULL,DEVICE_NAME);
      printk(DEVICE_NAME " initialized \n");
      return 0;
      fial_malloc:unregister_chrdev_region(dev_num, 1);
  }   
  
  
    /*卸载驱动程序*/
  static void buttons_exit(void)
  {
      cdev_del(buttons_cdev_p);//注销cdev
      kfree(buttons_cdev_p);//释放设备结构体内存
      unregister_chrdev_region(dev_num,DEV_COUNT);//释放设备号 
  }

            
  /*这两行制定驱动程序的初始化函数和卸载函数*/
  module_init(buttons_init);
  module_exit(buttons_exit);

  MODULE_LICENSE("GPL");
