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
		/*argc是命令行总的参数个数 
argv[]是argc个参数，其中第0个参数是程序的全名（包括文件绝对路径），以后的参数命令行后面跟的用户输入的参数，*/
		//sscanf返回值为1表示读到了一个有效数据，sscanf（）的返回为读有有效数据的个数！
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