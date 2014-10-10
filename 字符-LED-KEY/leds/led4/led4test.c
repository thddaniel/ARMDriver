#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

 
int	main(int argc, char **argv)
{
 	int on;
	int led_no;
	int fd;
	if (argc !=3 || sscanf(argv[1], "%d", &led_no) != 1 || sscanf(argv[2],"%d", &on) !=1 ||
		on < 0 || on >1 || led_no < 0 || led_no > 3)
	 {
	 	fprintf(stderr, "Usage: leds led_no 0|1\n");
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