#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>



int main()
{
	int fd;
	fd_set rds;
	int ret;
	char Buf[128];
	
	/*初始化Buf*/
	strcpy(Buf,"Mem is char dev!");
	printf("BUF: %s\n",Buf);
	
	/*打开设备文件*/
	fd = open("/dev/memdev0",O_RDWR);
	if (fd == NULL)
	{
		printf("Open Memdev Error!\n");
		return -1;
	}

	FD_ZERO(&rds);
	FD_SET(fd, &rds);
	
    strcpy(Buf,"Buf is NULL!");
	printf("Read BUF1: %s\n", Buf);

	ret = select(fd + 1, &rds, NULL, NULL, NULL);
	if (ret < 0)
	{
		printf("select error!\n");
		exit(1);
	}

	if(FD_ISSET(fd, &rds))
		read(fd, Buf, sizeof(Buf));

	printf("Read BUF2: %s\n", Buf);

	close(fd);
	
	return 0;	

}
