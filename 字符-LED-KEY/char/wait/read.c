#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int main()
{
	FILE *fp = NULL;
	char Buf[128];
	
	/*初始化Buf*/
	strcpy(Buf,"Mem is char dev!");
	printf("BUF: %s\n",Buf);
	
	/*打开设备文件*/
	fp = fopen("/dev/memdev0","r+");
	if (fp == NULL)
	{
		printf("Open Memdev Error!\n");
		return -1;
	}
	
    strcpy(Buf,"Buf is NULL!");
	printf("Read BUF1: %s\n", Buf);

	fread(Buf, sizeof(Buf), 1, fp);

	printf("Read BUF2: %s\n", Buf);

	fclose(fp);
	
	return 0;	

}
