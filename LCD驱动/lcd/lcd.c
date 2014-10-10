#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
//#include <asm/arch/regs-lcd.h>
//#include <asm/arch/regs-gpio.h>
//#include <asm/arch/fb.h>

#include<mach/regs-lcd.h>
#include<mach/regs-gpio.h>
#include<mach/fb.h>



static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info);

/*	每个寄存器地址相隔4*/
struct lcd_regs {
	unsigned long	lcdcon1;
	unsigned long	lcdcon2;
	unsigned long	lcdcon3;
	unsigned long	lcdcon4;
	unsigned long	lcdcon5;
    unsigned long	lcdsaddr1;
    unsigned long	lcdsaddr2;
    unsigned long	lcdsaddr3;
    unsigned long	redlut;
    unsigned long	greenlut;
    unsigned long	bluelut;
    unsigned long	reserved[9];
    unsigned long	dithmode;
    unsigned long	tpal;
    unsigned long	lcdintpnd;
    unsigned long	lcdsrcpnd;
    unsigned long	lcdintmsk;
    unsigned long	lpcsel;
};

static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= s3c_lcdfb_setcolreg,//假的调色板函数
	.fb_fillrect	= cfb_fillrect,		  //矩形填充
	.fb_copyarea	= cfb_copyarea,		  //数据复制
	.fb_imageblit	= cfb_imageblit,	  //图形填充
};


static struct fb_info *s3c_lcd;
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile struct lcd_regs* lcd_regs;
static u32 pseudo_palette[16];//假的调色板


/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	
	if (regno > 16)
		return 1;

	/* 用red,green,blue三原色构造出val */
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);
	
	//((u32 *)(info->pseudo_palette))[regno] = val;
	pseudo_palette[regno] = val;
	return 0;
}

static int lcd_init(void)
{
	/* 1. 分配一个fb_info */
	s3c_lcd = framebuffer_alloc(0, NULL);

	/* 2. 设置 */
	/* 2.1 设置固定的参数 */
	strcpy(s3c_lcd->fix.id, "mylcd");
	//s3c_lcd->fix.smem_len = 480*272*32/8;        /* TQ2440的LCD位宽是24(24bpp),但是2440里会分配4字节即32位(浪费1字节) */
	s3c_lcd->fix.smem_len = 320*240*32/8; 
	s3c_lcd->fix.type     = FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual   = FB_VISUAL_TRUECOLOR; /* TFT */
	//s3c_lcd->fix.line_length = 480*4;
	s3c_lcd->fix.line_length = 320*4;
	/* 2.2 设置可变的参数 */
	/*s3c_lcd->var.xres           = 480;
	s3c_lcd->var.yres           = 272;
	s3c_lcd->var.xres_virtual   = 480;
	s3c_lcd->var.yres_virtual   = 272;
	s3c_lcd->var.bits_per_pixel = 32;*/
	s3c_lcd->var.xres			= 320;
	s3c_lcd->var.yres			= 240;
	s3c_lcd->var.xres_virtual	= 320;
	s3c_lcd->var.yres_virtual	= 240;
	s3c_lcd->var.bits_per_pixel = 32;

	/* RGB:565 */
	s3c_lcd->var.red.offset     = 16;
	s3c_lcd->var.red.length     = 8;
	
	s3c_lcd->var.green.offset   = 8;
	s3c_lcd->var.green.length   = 8;

	s3c_lcd->var.blue.offset    = 0;
	s3c_lcd->var.blue.length    = 8;

	s3c_lcd->var.activate       = FB_ACTIVATE_NOW;
	
	
	/* 2.3 设置操作函数 */
	s3c_lcd->fbops              = &s3c_lcdfb_ops;
	
	/* 2.4 其他的设置 */
	s3c_lcd->pseudo_palette = pseudo_palette; 
	//s3c_lcd->screen_base  = ;  /* 显存的虚拟地址 */ 
	s3c_lcd->screen_size   = 320*240*32/8;

	/* 3. 硬件相关的操作 */
	/* 3.1 配置GPIO用于LCD */
	gpbcon = ioremap(0x56000010, 8);
	gpbdat = gpbcon+1;
	gpccon = ioremap(0x56000020, 4);
	gpdcon = ioremap(0x56000030, 4);
	gpgcon = ioremap(0x56000060, 4);

    *gpccon  = 0xaaaaaaaa;   /* GPIO管脚用于VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND */
	*gpdcon  = 0xaaaaaaaa;   /* GPIO管脚用于VD[23:8] */
	
//	*gpbcon &= ~(3);  /* GPB0设置为输出引脚 */
//	*gpbcon |= 1;
//	*gpbdat &= ~1;     /* 输出低电平 */

	*gpgcon |= (3<<8); /* GPG4用作LCD_PWREN */
	
	/* 3.2 根据LCD手册设置LCD控制器, 比如VCLK的频率等 */
	lcd_regs = ioremap(0x4D000000, sizeof(struct lcd_regs));

	/* 
	 * TQ2440 4.3英寸LCD手册为WXCAT43-TG6#001_V1.0.pdf第22、23页
	 * 
	 * LCD手册和2440手册"Figure 15-6. TFT LCD Timing Example"一对比就知道参数含义了
	 */

	/* bit[17:8]: VCLK = HCLK / [(CLKVAL+1) x 2], LCD手册P22 (Dclk=9MHz~15MHz)
	 *            10MHz(100ns) = 100MHz / [(CLKVAL+1) x 2]
	 *            CLKVAL = 4
	 * bit[6:5]: 0b11, TFT LCD
	 * bit[4:1]: 0b1101, 24 bpp for TFT
	 * bit[0]  : 0 = Disable the video output and the LCD control signal.
	 */
	lcd_regs->lcdcon1  = (4<<8) | (3<<5) | (0x0d<<1);

	/* 垂直方向的时间参数
	 * bit[31:24]: VBPD, VSYNC之后再过多长时间才能发出第1行数据
	 *             LCD手册 tvb=2
	 *            																			       VBPD=1                       //VBPD=14
	 * bit[23:14]: 多少行, 272, 所以                                                                                                 LINEVAL=272-1=271    // LINEVAL=240-1=239
	 * bit[13:6] : VFPD, 发出最后一行数据之后，再过多长时间才发出VSYNC
	 *             LCD手册tvf=2, 所以                                                                                                     VFPD=2-1=1               //12-1=11
	 * bit[5:0]  : VSPW, VSYNC信号的脉冲宽度, LCD手册tvp=10, 所以                                  VSPW=10-1=9            //  VSPW=3-1=2
	 */
	//lcd_regs->lcdcon2  = (1<<24) | (271<<14) | (1<<6) | (9<<0);
	lcd_regs->lcdcon2  = (14<<24) | (239<<14) | (11<<6) | (2<<0);
	/* 水平方向的时间参数
	 * bit[25:19]: HBPD, VSYNC之后再过多长时间才能发出第1行数据
	 *             LCD手册 thb=2
	 *                                                                                                                                                                                         	 HBPD=2-1=1               // HBPD=38-1=37 
	 * bit[18:8]: 多少列, 480, 所以                                                                                                                                       HOZVAL=480-1=479   //  HOZVAL=320-1=319
	 * bit[7:0] : HFPD, 发出最后一行里最后一个象素数据之后，再过多长时间才发出HSYNC
	 *             LCD手册thf=2, 所以                                                                                                                                         HFPD=2-1=1             //HFPD=20-1=19    
	 */
	//lcd_regs->lcdcon3 = (1<<19) | (479<<8) | (1<<0);
	lcd_regs->lcdcon3 = (37<<19) | (319<<8) | (19<<0);

	/* 水平方向的同步信号
	 * bit[7:0]	: HSPW, HSYNC信号的脉冲宽度, LCD手册Thp=41, 所以HSPW=41-1=40
	 */	
	//lcd_regs->lcdcon4 = 40;
    lcd_regs->lcdcon4 = 29;
	/* 信号的极性 
	 * bit[11]: 1=565 format, 对于24bpp这个不用设
	 * bit[10]: 0 = The video data is fetched at VCLK falling edge
	 * bit[9] : 1 = HSYNC信号要反转,即低电平有效 
	 * bit[8] : 1 = VSYNC信号要反转,即低电平有效 
	 * bit[6] : 0 = VDEN不用反转
	 * bit[3] : 0 = PWREN输出0   //   0 = 禁止PWREN 信号 1 = 允许PWREN 信号
	 *
	 * BSWP = 0, HWSWP = 0, BPP24BL = 0 : 当bpp=24时,2440会给每一个象素分配32位即4字节,哪一个字节是不使用的? 看2440手册P412
         * bit[12]: 0, LSB valid, 即最高字节不使用
	 * bit[1] : 0 = BSWP
	 * bit[0] : 0 = HWSWP
	 */
	lcd_regs->lcdcon5 = (0<<10) | (1<<9) | (1<<8) | (0<<12) | (0<<1) | (0<<0);
	


	/* 3.3 分配显存(framebuffer), 并把地址告诉LCD控制器 */
	s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->fix.smem_start, GFP_KERNEL);
	
	lcd_regs->lcdsaddr1  = (s3c_lcd->fix.smem_start >> 1) & ~(3<<30);
	lcd_regs->lcdsaddr2  = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1) & 0x1fffff;
	//lcd_regs->lcdsaddr3  = (480*32/16);  /* 一行的长度(单位: 2字节) */	
	lcd_regs->lcdsaddr3  = (320*32/16); 
	//s3c_lcd->fix.smem_start = xxx;  /* 显存的物理地址 */

	/* 启动LCD */
	lcd_regs->lcdcon1 |= (1<<0); /* 使能LCD控制器 */
	lcd_regs->lcdcon5 |= (1<<3); /* 使能LCD本身: LCD_PWREN */
//	*gpbdat |= 1;     /* 输出高电平, 使能背光, TQ2440的背光电路也是通过LCD_PWREN来控制的 */

	/* 4. 注册 */
	register_framebuffer(s3c_lcd);
	
	return 0;
}

static void lcd_exit(void)
{
	unregister_framebuffer(s3c_lcd);
	lcd_regs->lcdcon1 &= ~(1<<0); /* 关闭LCD控制器 */
	lcd_regs->lcdcon1 &= ~(1<<3); /* 关闭LCD本身 */
//	*gpbdat &= ~1;     /* 关闭背光 */
	dma_free_writecombine(NULL, s3c_lcd->fix.smem_len, s3c_lcd->screen_base, s3c_lcd->fix.smem_start);
	iounmap(lcd_regs);
	iounmap(gpbcon);
	iounmap(gpccon);
	iounmap(gpdcon);
	iounmap(gpgcon);
	framebuffer_release(s3c_lcd);
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");


