#include <stdio.h>

int main()
{
	FILE *fp0 = NULL;
	char Buf[4096];
	 
	/*初始化Buf*/
	strcpy(Buf, "Mem is char dev!");
	printf("BUF: %s\n",Buf);

	/*打开设备文件*/
	fp0 = fopen("/dev/memdev0","r+");
	if(fp0 == NULL)
	{
		printf("Open Memdev0 Error!\n");
		return -1;
	}

	/*写入设备*/
	fwrite(Buf, sizeof(Buf), 1, fp0);//

	/* 重新定位文件位置（思考没有该指令，会有何后果）*/
	fseek(fp0, 0, SEEK_SET);
	
	/*清除Buf*/
	strcpy(Buf, "Buf is NULL!");
	printf("BUF: %s\n",Buf);

	/*读出设备*/
	fread(Buf, sizeof(Buf), 1, fp0);
	//从一个文件流中读数据,读取1 个元素,
	//每个元素size字节.如果调用成功返回1.
	//如果调用成功则实际读取size*1字节 

	/*检测结果*/
	printf("Buf: %s\n", Buf);
}