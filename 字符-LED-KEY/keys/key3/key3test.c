 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <sys/ioctl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
  
  int main(int argc,char **argv)
  {
     int i;
     int ret;
     int fd;
     int press_cnt[4];
 
     fd=open("/dev/buttons",O_RDWR);
     if(fd<0)  
	 {
         printf("Can't open /dev/buttons \n");
         return -1;
     }
     //���Ǹ�����ѭ���������п�����read���������ߣ����а�������ʱ��
     //���ŷ���
     while(1) 
	 {
         ret = read(fd,press_cnt,sizeof(press_cnt));
         if(ret<0)  
		 {
             printf("read err !\n");
             continue;
         }
         //��������µĴ�����Ϊ0����ӡ���� 
        for(i=0;i<sizeof(press_cnt)/sizeof(press_cnt[0]);i++)    
	    {
              if(press_cnt[i])
                printf("Key%d has been pressed %d times \n",i+1,press_cnt[i]);
         }         
    }                
 }