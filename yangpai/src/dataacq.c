/*
 * dataacq.c
 *
 *  Created on: 2016-4-8
 *      Author: vmuser
 */
#include"dataacq.h"
#include"gpio.h"
#include"spi.h"
#include "data.h"
#include"i2cwork.h"
#include "readpara.h"
char acProcessName[32];

void sig_handler(int signo);

//extern i2cdata i2c;
/*
 * 以前是中断触发，改成程序自己记时触发
 */
void sigFunc()
{
 //  static int iCnt = 0;
   spi_work();
 //  i2c_work();
}
/*
 * 程序计时，一秒一次
 */
int dataacq(void)
{

	struct itimerval tv, otv;
	signal(SIGALRM, sigFunc);
	//how long to run the first time
	tv.it_value.tv_sec = 1;
	tv.it_value.tv_usec = 0;
	//after the first time, how long to run next time
	tv.it_interval.tv_sec = 1;
	tv.it_interval.tv_usec = 0;

	if (setitimer(ITIMER_REAL, &tv, &otv) != 0)
	printf("setitimer err\n");

	while(1)
	{
		sleep(1);
//		printf("otv: %d, %d, %d, %d\n", otv.it_value.tv_sec, otv.it_value.tv_usec, otv.it_interval.tv_sec, otv.it_interval.tv_sec);
	}
//	interrupt();//中断触发程序
	return 0;
}
int interrupt()//GPIO2_21    53号
{
	int gpio_fd, ret;
	struct pollfd fds[1];
	char buff[10];
	int inter_C=0;
//按键引脚初始化
	gpio_export(53);
	gpio_direction(53, 0);
	gpio_edge(53,1);
	gpio_fd = open("/sys/class/gpio/gpio53/value",O_RDONLY);
	if(gpio_fd < 0){
		MSG("Failed to open value!\n");
		return -1;
	}
	fds[0].fd = gpio_fd;
	fds[0].events  = POLLPRI;
	ret = read(gpio_fd,buff,10);
	if( ret == -1 )
		MSG("read\n");
	while(1){
		ret = poll(fds,1,0);
		if( ret == -1 )
			MSG("poll\n");
		if( fds[0].revents & POLLPRI){
			ret = lseek(gpio_fd,0,SEEK_SET);
			if( ret == -1 )
				MSG("lseek\n");
			ret = read(gpio_fd,buff,10);
			if( ret == -1 )
				MSG("read\n");
//			MSG("ok\n");
//			i2c_work();//暂时不读取adc数据
			inter_C++;
			if(inter_C==200)
			{
				spi_work();//spi暂时没有程序
				inter_C=0;
			}
			printf("inter_C=%d\n",inter_C);

//5ms数据存储，以后创建线程
//			write_5ms_data();//暂时不写数据库
		}
		usleep(1000);
	}
	return 0;
}

int intersignal(void)
{
	//system("/mnt/ad1115_datas &");
	if(signal(SIGUSR1,sig_handler) == SIG_ERR){
		perror("signal errror");
		exit(EXIT_FAILURE);
	}
	for(; ;);//有时间让我们发送信号

	return 0;
}
void sig_handler(int signo)
{
	spi_work();
    printf("\n ertyuiop\n");
    //exit(EXIT_FAILURE);
}
