#include <stdio.h>
#include <string.h>

int main()
{
	FILE *fp = NULL;
	char Buf[128];
		
	/*打开设备文件*/
	fp = fopen("/dev/memdev0","r+");
	if (fp == NULL)
	{
		printf("Open Memdev Error!\n");
		return -1;
	}
	
	strcpy(Buf, "memdev is char dev!");
	printf("Write BUF: %s\n", Buf);
	fwrite(Buf, sizeof(Buf), 1, fp);

	sleep(5);
	fclose(fp);
	
	return 0;	
}
