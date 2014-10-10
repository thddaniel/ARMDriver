#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

 

 
/***************************************************************************/
/* Function name : int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);*/                                        
/* Description   : 设置波特率，数据位，奇偶校验位，停止位              	  */
/* Return type   ：int                                                 
//*Argument      : fd:打开文件 speed:波特率 bit:数据位  neent:奇偶校验位 stop:停止位 */
/***************************************************************************/
 int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{

    struct termios newtio,oldtio;
    if ( tcgetattr( fd,&oldtio) != 0) { 
        perror("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag |= CLOCAL | CREAD; 
    newtio.c_cflag &= ~CSIZE; 
    newtio.c_oflag &= ~(ONLCR | OCRNL);  
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);   

	
    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
    break;
    case 8:
        newtio.c_cflag |= CS8;
    break;
    }
	
	
    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E': 
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N': 
        newtio.c_cflag &= ~PARENB;
        break;
    }
	
	
    switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
         cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if( nStop == 1 )
        newtio.c_cflag &= ~CSTOPB;
    else if ( nStop == 2 )
    newtio.c_cflag |= CSTOPB;
    newtio.c_cc[VTIME] =5; //测试时该大一点
    newtio.c_cc[VMIN] = 0;//set min read byte!
	//newtio.c_cc[VSUSP] = 0;
	

    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}


 
/****************************************************************************/
/* Function name : int open_port(int fd,int comport);              */
/* Description   : 打开串口0/1/2              					   */
/* Return type   ：int                                             */
//* Argument      : fd:打开文件  comport:打开的端口				   */                                          */
/****************************************************************************/
int open_port(int fd,int comport)
{
    char *dev[]={"/dev/tanghao2440_serial0","/dev/tanghao2440_serial1","/dev/tanghao2440_serial2"};
    long vdisable;
    if (comport==0)
    {    fd = open( "/dev/tanghao2440_serial0", O_RDWR|O_NOCTTY|O_NDELAY);
        if (-1 == fd){
            perror("Can't Open Serial Port");
            return(-1);
        }
        else 
            printf("open tanghao2440_serial0 .....\n");
    }
    else if(comport==1)
    {    fd = open( "/dev/tanghao2440_serial1", O_RDWR|O_NOCTTY|O_NDELAY);

    if (-1 == fd){
        perror("Can't Open Serial Port");
            return(-1);
        }
        else 
            printf("open tanghao2440_serial1 .....\n");
    }
    else if (comport==2)
    {
        fd = open( "/dev/tanghao2440_serial2", O_RDWR|O_NOCTTY|O_NDELAY);
        if (-1 == fd){
            perror("Can't Open Serial Port");
            return(-1);
        }
        else 
            printf("open tanghao2440_serial2 .....\n");
    }
    if(fcntl(fd, F_SETFL, 0)<0)
        printf("fcntl failed!\n");
    else
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
    if(isatty(STDIN_FILENO)==0)
        printf("standard input is not a terminal device\n");
    else
        printf("isatty success!\n");
    printf("fd-open=%d\n",fd);
    return fd;
}



int main(int argc, char **argv)
{
	int fd;
	int i;
	int nread=0;
	int nwrite=0;

    /*  
	char buf[] = "AT+CMGF=1";
	char buf1 = 0x0d;
	char buf2[] = "AT+CMGS=";
	//char buf2_1 ='\"';
	char buf2_2[] = "\"13427305491\"";
	char buf3[] = "hello,ni hao";
	//char buf4 = 0x1A;
	//char buf4[] ="Ctrl-z";
  	*/

	if(( fd=open_port(fd,0)) < 0){
   		perror("open_port error");
   		return -1;
	}

 	if(( i=set_opt(fd,9600,8,'N',1) ) < 0){
   		perror("set_opt error");
   		return -1;
	}

	/*
	nwrite = write(fd,buf,sizeof(buf));
	nwrite = write(fd,buf1, sizeof(buf1));
	sleep(1);
	
	nwrite = write(fd,buf2,sizeof(buf2));
	//nwrite = write(fd,buf2_1,sizeof(buf2_1));
	nwrite = write(fd,buf2_2,sizeof(buf2_2));
	//nwrite = write(fd,buf2_1,sizeof(buf2_1));
	nwrite = write(fd,buf1, sizeof(buf1));
	sleep(1);
	
	nwrite = write(fd,buf3,sizeof(buf3));
	//nwrite = write(fd,buf4,sizeof(buf4));
    nwrite = write(fd,"\x1a",1);
	sleep(1);
	*/

	//打电话
	//nwrite = write(fd,"ATD13750362962;\r",16);
	//sleep(1);

 
	//发短信
	nwrite = write(fd,"AT\r",sizeof("AT\r"));
	usleep(100000);
	nwrite = write(fd,"AT+CMGF=1\r",sizeof("AT+CMGF=1\r")); 
	usleep(100000);
	nwrite = write(fd,"AT+CSMP=17,167,0,0\r",sizeof("AT+CSMP=17,167,0,0\r"));
	usleep(100000);
	nwrite = write(fd,"AT+CSCS=GSM\r", sizeof("AT+CSCS=GSM\r"));
	usleep(100000);
	nwrite = write(fd,"AT+CMGS=\"13750362962\"\r",sizeof("AT+CMGS=\"13750362962\"\r"));
	usleep(50000);
	nwrite = write(fd,"hello\x1A",sizeof("hello\x1A"));
	usleep(50000);
	 

	/*
	nwrite = write(fd, "AT+CMGF=1\r", sizeof("AT+CMGF=1\r"));
	usleep(100000);
	nwrite = write(fd, "AT+CMGS=\"13750362962\"\r", sizeof("AT+CMGS=\"13750362962\"\r"));
	usleep(50000);
	nwrite = write(fd,"hello\x1a",sizeof("hello\x1a"));
	sleep(2);
	*/

	close(fd);
    return 0;	
}
