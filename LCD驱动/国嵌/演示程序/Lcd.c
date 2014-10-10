#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

int main () {
    int fp=0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long screensize=0;
    char *fbp = 0;
    int x = 0, y = 0;
    long location = 0;
    fp = open ("/dev/fb0",O_RDWR);

    if (fp < 0){
        printf("Error : Can not open framebuffer device\n");
        exit(1);
    }

    if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo)){
        printf("Error reading fixed information\n");
        exit(2);
    }
    if (ioctl(fp,FBIOGET_VSCREENINFO,&vinfo)){
        printf("Error reading variable information\n");
        exit(3);
    }

    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;   //��֡����ռ�
    /*����ǰ�fp��ָ���ļ��дӿ�ʼ��screensize��С�����ݸ�ӳ��������õ�һ��ָ�����ռ��ָ��*/
    fbp =(char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp,0);
    if ((int) fbp == -1)
    {
         printf ("Error: failed to map framebuffer device to memory.\n");
         exit (4);
    }
    /*�������뻭�ĵ��λ������,(0��0)������Ļ���Ͻ�*/
    for(x=100;x<150;x++)
   {
        for(y=100;y<150;y++)
       {
             location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;

             *(fbp + location) = 255;  /* ��ɫ��ɫ�� */  /*ֱ�Ӹ�ֵ���ı���Ļ��ĳ�����ɫ*/
             *(fbp + location + 1) = 0; /* ��ɫ��ɫ��*/   /*ע�����⼸����ֵ�����ÿ�������ֽ������õģ�������ÿ����2�ֽڣ�*/
             *(fbp + location + 2) = 0; /* ��ɫ��ɫ��*/   /*����RGB565������Ҫ����ת��*/
             *(fbp + location + 3) = 0;  /* �Ƿ�͸��*/ 
         } 
    }
    munmap (fbp, screensize); /*���ӳ��*/
    close (fp);    /*�ر��ļ�*/
    return 0;

}
