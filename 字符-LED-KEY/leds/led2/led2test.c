#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

 
int	main(int argc, char **argv)
{
 	int on;
	int led_no;
	int fd;
	if (argc !=3 || sscanf(argv[2], "%d", &led_no) != 1 || sscanf(argv[1],"%d", &on) !=1 ||
		on < 0 || on >1 || led_no < 0 || led_no > 3)
		/*argc���������ܵĲ������� 
argv[]��argc�����������е�0�������ǳ����ȫ���������ļ�����·�������Ժ�Ĳ��������к�������û�����Ĳ�����*/
		//sscanf����ֵΪ1��ʾ������һ����Ч���ݣ�sscanf�����ķ���Ϊ������Ч���ݵĸ�����
	 {
	 	fprintf(stderr, "Usage: leds 0|1 led_no \n");
		exit(1);
	 }
	 fd = open("/dev/leds", 0);
	 if(fd < 0)
	 {
	 	perror("open device leds");
		exit(1);
	 }
	 ioctl(fd, on,led_no );
	 close(fd);
	return 0;
}