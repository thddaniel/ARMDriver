#include <stdio.h>

int main()
{
	FILE *fp0 = NULL;
	char Buf[4096];
	 
	/*��ʼ��Buf*/
	strcpy(Buf, "Mem is char dev!");
	printf("BUF: %s\n",Buf);

	/*���豸�ļ�*/
	fp0 = fopen("/dev/memdev0","r+");
	if(fp0 == NULL)
	{
		printf("Open Memdev0 Error!\n");
		return -1;
	}

	/*д���豸*/
	fwrite(Buf, sizeof(Buf), 1, fp0);//

	/* ���¶�λ�ļ�λ�ã�˼��û�и�ָ����кκ����*/
	fseek(fp0, 0, SEEK_SET);
	
	/*���Buf*/
	strcpy(Buf, "Buf is NULL!");
	printf("BUF: %s\n",Buf);

	/*�����豸*/
	fread(Buf, sizeof(Buf), 1, fp0);
	//��һ���ļ����ж�����,��ȡ1 ��Ԫ��,
	//ÿ��Ԫ��size�ֽ�.������óɹ�����1.
	//������óɹ���ʵ�ʶ�ȡsize*1�ֽ� 

	/*�����*/
	printf("Buf: %s\n", Buf);
}