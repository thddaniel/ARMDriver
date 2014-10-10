#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ioctl.h"

int main()
{
 	int fd = 0;
	int cmd;
	int arg = 0;
	char Buf[4096];

	fd = open("/dev/memdev0", O_RDWR);
	if (fd < 0)
	{
		printf("Open Dev Mem0 Error!\n");
		return -1;
	}

	printf("<---Call MEMDEV_IOCPRINT--->\n");
	cmd = MEMDEV_IOCPRINT;
	if (ioctl(fd, cmd, &arg) < 0)
	{
		printf("Call cmd MEMDEV_IOCPIRNT fail\n");
		return -1;
	}

	printf("<--- Call MEMDEV_IOCSETDATA --->\n");
	cmd = MEMDEV_IOCSETDATA;
	arg = 2007;
	if (ioctl(fd, cmd, &arg) < 0)
	{
		printf("Call cmd MEMDEV_IOCSETDATA fail\n");
		return -1;
	}

	printf("<--- Call MEMDEV_IOCGETDATA --->\n");
	cmd = MEMDEV_IOCGETDATA;
	if (ioctl(fd, cmd, &arg) < 0)
	{	
		 printf("Call cmd MEMDEV_IOCGETDATA fail\n");
		 return -1;
	}
	printf("<--- In User Space MEMDEV_IOCGETDATA Get Data is %d--->\n\n", arg);

	close(fd);	
	return 0;	
}
