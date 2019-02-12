/*
 * spi.c
 *
 *  Created on: 2016-4-8
 *      Author: vmuser
 */
#include<mysql.h>
#include"spi.h"
#include"flow_calculation.h"
#include "Fcalculate.h"
#include "spmath.h"
#include "spi.h"
#include "data.h"
#include "i2cwork.h"
#include "curvedata.h"
#include "readpara.h"
#include "shmdata.h"

static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 2000000;
static uint16_t delay;

#define SPI_DEBUG 0
unsigned int es[8192];
extern spiset 			spisets;
extern spidata 			spidatas;
extern int 			win_C[200][6];//计算用的二维数组
extern flag				flags;
//脱离线程外部变量
extern pthread_attr_t 	thread_attr;
extern pthread_attr_t 	thread_attr1;
extern pthread_attr_t 	thread_attr2;
extern pthread_attr_t 	thread_attr3;
extern pthread_t 		a_thread;
extern pthread_t 		a_thread1;
extern pthread_t 		a_thread2;
extern pthread_t 		a_thread3;
extern Adjust			ParaAdjust;

extern struct shared_use_st *shared;//指向shm

//脱离线程能普运行程序
void *thread_function(void){
	sleep(2);
//能谱计算--调压----能谱拉伸/卷积调窗
	//es_adjust1();
	errorCode();
	spectral_data_w();//能普数据存入json

	if(flags.flag_es!=0){
		write_es_data();//能普写入数据库
		//shared->W_Es=1;//能普写入标志位
	}
	printf("%f\n",shared->S_HV_C);
	system("/mnt/adjustHV &");
	pthread_exit(NULL);
	return 0;
}
//脱离线程存储脉冲数据
void *thread_function1(void){
	sleep(0.5);
	//printf("pules w\n");
	pulse_data_w();//脉冲数据存入json
	pthread_exit(NULL);
	return 0;
}
//脱离线程计算流量
void *thread_function2(void){

	arm_P();
	//;//数据整合
	fcalculate();//数据计算
	//write_logfile(void)//数据存储需要更改
	if(flags.flag_5ms!=0){
		write_5ms_data();//5ms数据存储
	}
	/*if(shared->W_log==0&&shared->W_Es==1){
		printf("ES------W\n");
		write_es_data();//能普写入数据库
		shared->W_Es=0;
	}
	if(shared->W_log==1){
		printf("log------W\n");
		shared->W_log=0;
		W_logfile();
	}*/

	pthread_exit(NULL);
	return 0;
}

//脱离线程流量计算程序一秒钟算一下
void *thread_function3(void *arg){
	hander_result* ptrs;
	ptrs = arg;
	sleep(0.3);
	getResult(ptrs->LlinearatTenuation,ptrs->HlinearatTenuation);
	pthread_exit(NULL);
	return 0;
}
static int getCS(void)
{
	spiW(2, 2, 0x1e, 0x00);//le
	spiR(0, 2, 0x00, 0x00);
	return 0;
}
static int getCD(void)
{
	spiW(2, 2, 0x10, 0x00);//le
	spiR(1, 2, 0x00, 0x00);
	return 0;
}
static int getfifo(void)
{
	spiW(2, 2, 0x0e, 0x00);//le
	spiR(2, 2, 0x00, 0x00);
	return 0;
}
static int getLe(void)
{
	spiW(2, 2, 0x01, 0x00);//le
	spiR(1, 2, 0x00, 0x00);
//	printf("%d,", pt->spidata[0]*256+pt->spidata[1]);
	return 0;
}
static int getHe(void)
{
	spiW(2, 2, 0x02, 0x00);
	spiR(2, 2, 0x00, 0x00);
//	printf("%d,", pt->spidata[2]*256+pt->spidata[3]);
	return 0;
}
static int get356(void)
{
	spiW(2, 2, 0x07, 0x00);
	spiR(5, 2, 0x00, 0x00);
//	printf("%d,", pt->spidata[4]*256+pt->spidata[5]);
	return 0;
}
static int EsStart(void)
{
	spiW(2, 2, 0x1D, 0x00);
	spiR(0, 2, 0x00, 0x00);
	return 0;
}
static int EsCollect(void)
{
	spiW(2, 2, 0x03, 0x00);
	spiR(6, 2, 0x00, 0x00);
//	printf("%d,", pt->spidata[10]*256+pt->spidata[11]);
	return 0;
}
static int PulseStart(void)
{
	spiW(2, 2, 0x1C, 0x00);
	spiR(0, 2, 0x00, 0x00);
	return 0;
}
static int PulseState(void)
{
	spiW(2, 2, 0x0E, 0x00);
	spiR(7, 2, 0x00, 0x00);
//	printf("%d,", pt->spidata[12]*256+pt->spidata[13]);
	return 0;
}
static int PulseData(void)
{
	spiW(2, 2, 0x0F, 0x00);
	spiR(8, 2, 0x00, 0x00);
//	printf("%d,", pt->spidata[14]*256+pt->spidata[15]);
	return 0;
}
static int datasum (hander_result * pt)//整理计算数据
{

	return 0;
}
static void lnhtlt(void)
{
	int i,j;
	int k=0;
	int sumA=0;
	int sumB=0;
	int sumC=0;
	int sumD=0;
	int sumE=0;
	float aveA=0;
	float aveB=0;
	float aveC=0;
	float aveD=0;
	float aveE=0;
	int a=0;
	int b=0;
	int c=0;
	int d=0;
	int e=0;
	int win[1200];

	static int cn_c;
	if(cn_c==1)
	{
		for(i=0;i<1200;i++)
		{
			win_C[i/6][i%6] = -1;
		}
		win[0]=0xaa55;

		for(i=0;i<200;i++)
		{
			getfifo();
		//	printf("%.4x&&&&&&\n",spidatas.k81);
			if(spidatas.k81&0x20){
				break;
			}
		}
		for(i=0;i<200;i++){
			getCD();
		//	printf("%.4x,%.4x\n",spidatas.k81,spidatas.k31);
			if(spidatas.k31==0xaa55){
				break;
			}
		}
		for(i=0;i<1199;i++)
		{
			getCD();
			win[i+1]=spidatas.k31;
		//	win_C[i/6][i%6] = spidatas.k31;
		//	printf("%.4x,%.4x\n",spidatas.k81,spidatas.k31);
		}

		for(k=0,i=0;k<200;k++)
		{
			if(win[i++]==0xaa55){
				for(j=0;j<5;j++)
				{
					if(win[i]!=0xaa55)
					{
						//printf("%d\n",win[i]);
						win_C[k][j]=win[i++];

					}
					else
					{
						i--;
						break;
					}
				}
			}
		}

		for(i=0;i<99;i++){
			if(win_C[i][0]<1000)
			{
				sumA=sumA+win_C[i][0];
				a++;
			}
		}
		aveA=sumA/a;
		if(aveA==0)
		{
			aveA=1;
		}
		for(i=0;i<200;i++)
		{
	//		if(win_C[i][0]==0)
	//			win_C[i][0]=1;
			if(win_C[i][0]>=1000)
				win_C[i][0]=aveA;
			if(win_C[i][0]>(aveA*10))//||win_C[i][0]<(aveA*0.1))
				win_C[i][0]=aveA;
		}

		for(i=0;i<99;i++){
			if(win_C[i][1]<1000)
			{
				sumB=sumB+win_C[i][1];
				b++;
			}
		}
		aveB=sumB/b;
		if(aveB==0)
		{
			aveB=1;
		}
		for(i=0;i<200;i++)
		{
	//		if(win_C[i][1]==0)
		//		win_C[i][1]=1;
			if(win_C[i][1]>=1000)
				win_C[i][1]=aveB;
			if(win_C[i][1]>(aveB*10))//||win_C[i][1]<(aveB*0.1))
				win_C[i][1]=aveB;
		}

		for(i=0;i<99;i++){
			if(win_C[i][2]<1000)
			{
				sumC=sumC+win_C[i][2];
				c++;
			}
		}
		aveC=sumC/c;
		if(aveC==0)
		{
			aveC=1;
		}
		for(i=0;i<200;i++)
		{
	//		if(win_C[i][2]==0)
		//		win_C[i][2]=1;
			if(win_C[i][2]>=1000)
				win_C[i][2]=aveC;
			if(win_C[i][2]>(aveC*10))//||win_C[i][2]<(aveC*0.1))
				win_C[i][2]=aveC;
		}

		for(i=0;i<99;i++){
			if(win_C[i][3]<1000)
			{
				sumD=sumD+win_C[i][3];
				d++;
			}
		}
		aveD=sumD/d;
		if(aveD==0)
		{
			aveD=1;
		}
		for(i=0;i<200;i++)
		{
	//		if(win_C[i][3]==0)
		//		win_C[i][3]=1;
			if(win_C[i][3]>=1000)
				win_C[i][3]=aveD;
			if(win_C[i][3]>(aveD*10))//||win_C[i][3]<(aveD*0.1))
				win_C[i][3]=aveD;
		}

		for(i=0;i<99;i++){
			if(win_C[i][4]<1000)
			{
				sumE=sumE+win_C[i][4];
				e++;
			}
		}
		aveE=sumE/e;
		if(aveE==0)
		{
			aveE=1;
		}
		for(i=0;i<200;i++)
		{
	//		if(win_C[i][4]==0)
		//		win_C[i][4]=1;
			if(win_C[i][4]>=1000)
				win_C[i][4]=aveE;
			if(win_C[i][4]>(aveE*10))//||win_C[i][4]<(aveE*0.1))
				win_C[i][4]=aveE;
		}


/*		for(i=0;i<199;i++)
		{
			if(win_C[i][0]>=1000)
				win_C[i][0]=win_C[i+1][0];
			if(win_C[i][1]>=1000)
				win_C[i][1]=win_C[i+1][1];
			if(win_C[i][2]>=1000)
				win_C[i][2]=win_C[i+1][2];
			if(win_C[i][3]>=1000)
				win_C[i][3]=win_C[i+1][3];
			if(win_C[i][4]>=1000)
				win_C[i][4]=win_C[i+1][4];
		}
		if(win_C[199][0]>=1000)
			win_C[199][0]=win_C[198][0];
		if(win_C[199][1]>=1000)
			win_C[199][1]=win_C[198][1];
		if(win_C[199][2]>=1000)
			win_C[199][2]=win_C[198][2];
		if(win_C[199][3]>=1000)
			win_C[199][3]=win_C[198][3];
		if(win_C[199][4]>=1000)
			win_C[199][4]=win_C[198][4];*/



		//printf("%.4x,%.4x,%.4x,%.4x\n",win_C[0][5],win_C[198][5],win_C[199][5],win_C[54][5]);
		//开一个线程去计算每秒数据 计算200次 算完数据要存下来//开一个线程去存储5ms数据
		/////////////////////////////////////////////////创建一个脱离线程用于存储脉冲数据
			int res;
			res = pthread_create(&a_thread2,&thread_attr2,thread_function2,NULL);
			if(res != 0){
				perror("failed 3");
				exit(EXIT_FAILURE);
			}
			//属性用完后回收清理
						(void)pthread_attr_destroy(&thread_attr2);
			/////////////////////////////////////////////////////////////////////////////
	}
	if(cn_c==0)//开一个线程去读取数据
	{
		//system("/mnt/ad1115_datas &");
		getCS();
		cn_c=1;
		//system("/mnt/i2c_bus &");

	}
//	getLe();
//	getHe();
//	get356();
//	datasum();

}
void spiW(int tx_len, int rx_len ,unsigned char stx1, unsigned char stx2)
{
	uint8_t tx[] = {0,};
	tx[0] = stx1;
	tx[1] = stx2;

	int ret;
	struct spi_ioc_transfer tr_txrxw = {

	                .tx_buf = (unsigned long)tx,
	                .rx_buf = 0,
	                .len = tx_len,
	                .delay_usecs = delay,
	                .speed_hz = speed,
	                .bits_per_word = bits,
		};
	ret = ioctl(spidatas.fd_spi, SPI_IOC_MESSAGE(1), &tr_txrxw);
//	printf("#####");
}
void spiR(int tx_len, int rx_len ,unsigned char stx1, unsigned char stx2)
{
	uint8_t tx[] = {0,};
	tx[0] = stx1;
	tx[1] = stx2;
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	int ret;
	struct spi_ioc_transfer tr_txrx2 = {
					.tx_buf = 0,
					.rx_buf = (unsigned long)rx,
					.len = rx_len,
					.delay_usecs = delay,
					.speed_hz = speed,
					.bits_per_word = bits,
		};
	ret = ioctl(spidatas.fd_spi, SPI_IOC_MESSAGE(1), &tr_txrx2);
	if (ret == 1) {
		printf("can't revieve spi message");
	}
	switch(tx_len)
	   {
	   	case 1:
	   		spidatas.k31 = rx[0]*256+rx[1];
		    break;
	    case 2:
	    	spidatas.k81 = rx[0]*256+rx[1];
			break;
		case 3:
			spidatas.k160 = rx[0]*256+rx[1];
		    break;
	   	case 4:
	   		spidatas.k302 = rx[0]*256+rx[1];
		    break;
	    case 5:
	    	spidatas.k356 = rx[0]*256+rx[1];
			break;
	    case 6:
	    	spidatas.es = rx[0]*256+rx[1];
			break;
	    case 7:
	    	spidatas.pulF = rx[0]*256+rx[1];
	    	break;
	    case 8:
	    	spidatas.pul = rx[0]*256+rx[1];
	    	break;
		default :
		        break;
	   }
//	for (ret = 0; ret < tr_txrx2.len; ret++){
//		printf("%.2X ", rx[ret]);
//		pt->spidata[tx_len] = rx[ret];
//		tx_len++;
//	}
//	printf("%d,", rx[0]*256+rx[1]);
//	close(fd1);
}
/**
* 功 能：同步数据传输
* 入口参数 ：
* TxBuf -> 发送数据首地址
* len -> 交换数据的长度
* 出口参数：
* RxBuf -> 接收数据缓冲区
* 返回值：0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Transfer(const uint8_t *TxBuf, uint8_t *RxBuf, int len)
{
	int ret;

	struct spi_ioc_transfer tr ={
			.tx_buf = (unsigned long) TxBuf,
			.rx_buf = (unsigned long) RxBuf,
			.len =len,
			.delay_usecs = delay,
	};


	ret = ioctl(spidatas.fd_spi, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		printf("can't send spi message");
	else
	{
	#if SPI_DEBUG
		int i;
		printf("nsend spi message Succeed");
		printf("nSPI Send [Len:%d]: ", len);
		for (i = 0; i < len; i++)
		{
			if (i % 8 == 0)
				printf("\n");
			printf("0x%02X ", TxBuf[i]);
		}
		printf("\n");


		printf("SPI Receive [len:%d]:", len);
		for (i = 0; i < len; i++)
		{
			if (i % 8 == 0)
				printf("\n");
			printf("0x%02X ", RxBuf[i]);
		}
		printf("\n");
	#endif
	}
	return ret;
}

void setDefaultTh(void)
{
	int a;
	a = spisets.pch;
	a=a+8192;
//	SetTH(0x06,a);

//	a = spisets.th;//360
//	a=a+8192;
//		SetTH(0x09,a);

/*	spiW(2, 2, 0x93, 0xe8);
	spiW(2, 2, 0x94, 0x03);

	spiW(2, 2, 0x81, 0x10);//8000
	spiW(2, 2, 0x82, 0x27);*/
	SetTH(0x09,spisets.th);
	printf("th = %d\n",spisets.th);

	SetTH(0x00,spisets.bl);
	printf("Base Line = %d\n",spisets.bl);

	SetTH(0x01,spisets.winA1*2);
	SetTH(0x02,spisets.winA2*2);

	SetTH(0x03,spisets.winB1*2);
	SetTH(0x04,spisets.winB2*2);

	SetTH(0x08,spisets.winC1*2);
	SetTH(0x0e,spisets.winC2*2);

	SetTHS(0x1f,spisets.winD1*2);
	SetTHS(0x21,spisets.winD2*2);

	SetTHS(0x23,spisets.winE1*2);
	SetTHS(0x25,spisets.winE2*2);


/*
	a = spisets.winA1;//pt->TH0;
//	a=a+512*8;
	SetTH(0x01,a);

	a = spisets.winA2;//pt->TH1;
	SetTH(0x02,a);

	a = spisets.winB1;//pt->TH2;
	SetTH(0x03,a);

	a = spisets.winB2;//pt->TH3;
	SetTH(0x04,a);

	a = spisets.winC1;//pt->TH4;
	SetTH(0x08,a);*/

	a = spisets.st;//pt->st;
	SetTH(0x05,5);

	a = spisets.ct;//pt->ct;
	SetTH(0x07,30000);

	a = spisets.pth;//pt->pulse_thr;
	printf("pth = %d\n",spisets.pth);
	SetTH(0x0d,spisets.pth);

//	spiW(2, 2, 0x93, 0xe8);
	//spiW(2, 2, 0x94, 0x03);

	a = spisets.pmin;//pt->pulse_min;
	SetTH(0x0b,spisets.pmin);

	a = spisets.pmax;//pt->pulse_max;
	SetTH(0x0c,spisets.pmax);
}
void SetTH(char num,unsigned int Value)
{

	uint8_t add1,add2,data1,data2;
	add1 =(uint8_t) (0x81+num*2);
	add2 =(uint8_t) (0x82+num*2);
	data1 = (uint8_t) (0x00ff & Value);
	data2 = (uint8_t) (Value>>8);
//	printf("%.2X,%.2X,%.2X,%.2X ", add1,data1,add2,data2);

	spiW(2, 2, add1, data1);

	spiW(2, 2, add2, data2);
}
void SetTHS(char num,unsigned int Value)
{

	uint8_t add1,add2,data1,data2;
	add1 =(uint8_t) (0x81+num);
	add2 =(uint8_t) (0x82+num);
	data1 = (uint8_t) (0x00ff & Value);
	data2 = (uint8_t) (Value>>8);
//	printf("%.2X,%.2X,%.2X,%.2X ", add1,data1,add2,data2);

	spiW(2, 2, add1, data1);

	spiW(2, 2, add2, data2);
}

int spi()
{
	static const char *device = "/dev/spidev1.0";
	//static const char *device = "/dev/spidev1.1";

	int ret1 = 0;
	int fd1;


	fd1 = open(device, O_RDWR);
	if (fd1 < 0)
		printf("can't open device");


	ret1 = ioctl(fd1, SPI_IOC_WR_MODE, &mode);//数据输入 模式0
	if (ret1 == -1)
		printf("can't set wr spi mode");

	ret1 = ioctl(fd1, SPI_IOC_RD_MODE, &mode);//数据发送 模式0
	if (ret1 == -1)
		printf("can't get spi mode");


	ret1 = ioctl(fd1, SPI_IOC_WR_BITS_PER_WORD, &bits);//设置每字长度8字节
	if (ret1 == -1)
		printf("can't set bits per word");

	ret1 = ioctl(fd1, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret1 == -1)
		printf("can't get bits per word");


	ret1 = ioctl(fd1, SPI_IOC_WR_MAX_SPEED_HZ, &speed);//输入命令
	if (ret1 == -1)
		printf("can't set max speed hz");

	ret1 = ioctl(fd1, SPI_IOC_RD_MAX_SPEED_HZ, &speed);//输出命令
	if (ret1 == -1)
		printf("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	return fd1;
}

int spi_work(void)
{
//	printf("text1\n");
	readtime();
	lnhtlt();
	spectral_rd();
	pulse_rd();//以前5ms的读取方式；ok ok ok

	/*
	 * 是否调窗
	 */
	if(shared->F_WIN_J == 1){
		shared->F_WIN_J = 0;
		adjust_win();
	}
	return 0;
}
int spi_pulse(void)
{
//	uint8_t pulse_TX[1024]={0};
	uint8_t pulse_RX[2048]={0};
	static int pulse_F=1;
	if(pulse_F)
	{
		PulseStart();
		pulse_F=0;
	}
	else
	{
		SPI_Transfer(pulse_TX, pulse_RX, 10);
		pulse_F=1;
	}
	return 0;
}
int pulse_rd(void)
{
	static int pulse_en;
	static int pulse_sta;
	static int pulse_data;
	static int m;
	int i;
	int pulse_flag;
	if(pulse_en == 0){
		PulseStart();
		pulse_en = 1;
		pulse_sta = 1;
	}
	if(pulse_sta == 1){
		for(i=0;i<60;i++)
		{
			PulseState();
			pulse_flag = spidatas.pulF;
			//printf("@@@@%d\n",pulse_flag);
			if(pulse_flag&0x01){
				pulse_sta = 0;
				pulse_data = 1;
				break;
			}
		}

	}
	if(pulse_data == 1)
	{
		for(i=0;i<512;i++){
			PulseData();
			PulseState();

			pulse_flag = spidatas.pulF;

			spidatas.dpul[m]=spidatas.pul;
			//printf("%d,%d\n",i,spidatas.pul);
			m++;
		}

	pulse_en = 0;
	pulse_data = 0;
	m=0;

/////////////////////////////////////////////////创建一个脱离线程用于存储脉冲数据
	int res;
	res = pthread_create(&a_thread1,&thread_attr1,thread_function1,NULL);
	if(res != 0){
		perror("failed 3");
		exit(EXIT_FAILURE);
	}
	//printf("text");
//属性用完后回收清理
			(void)pthread_attr_destroy(&thread_attr1);
/////////////////////////////////////////////////////////////////////////////
	}
//	PulseData(fd1,pt);
	return 0;
}
int spectral_rd(void)
{
	int i;
	static int es_c;
	static int es_cn;
	static unsigned int dess[8192];
	static int spectral_c;
	if(spectral_c==1)
			EsStart();

	if(spectral_c >=30 && spectral_c <= 45)
	{
		for(i=0;i<512;i++)
		{
			EsCollect();
			//printf("%d,%d\n",es_c,spidatas.es);
			spidatas.des[es_c++] = spidatas.es;

		}
	}
	if(spectral_c == 45)
	{

		spectral_c = 0;
		es_c =0;
		es_cn++;
		for(i=0;i<8192;i++)
		{
			dess[i]=spidatas.des[i]+dess[i];

		}

	//	printf("#$##%d,%d\n",dess[1500],spisets.pch);

		if(es_cn==spisets.pch)
		{
			for(i=0;i<8192;i++)
			{
				spidatas.des[i]	=dess[i];
//				shared->S_es[i]	=dess[i];//原始能普误码太多，用去除误码后的能普
				es[i]			=dess[i];
				dess[i]			=0;
				//spidatas.des[i]=65535;
			}
			es_cn=0;
//printf("&&\n");
			//创建一个脱离线程用于存储能普数据
			int res;
			res = pthread_create(&a_thread,&thread_attr,thread_function,NULL);
			if(res != 0){
				perror("failed 3");
				exit(EXIT_FAILURE);
			}
			//属性用完后回收清理
			(void)pthread_attr_destroy(&thread_attr);
			/////////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}

	spectral_c++;
	return 0;
}
/*
 * 调整高压第一步：寻找能普峰位
 */
int es_adjust1(void)
{
	int i;
	int j;
	int k=0;
	int m;
	int a;
	static long int sum100;
	static long int sum100s;
	static int add;
	static int sub;
	int peak[10];
	unsigned int peakC[10];
	int peaks[10];
	unsigned int peakCs[10];
////	if(1)
//	{
		for(i=0;i<8090;i++){
			for(j=0;j<100;j++){
				sum100 = sum100 + spidatas.des[i+j];
			}
			if(sum100>sum100s){
				add++;
			}
			else{
				sub++;
			}
			if(add < sub)
			{
				add = 0;
				sub = 0;
			}
			//printf("%d++%d--\n",add,sub);
			if(add > 50 && sub >50){
				add = 0;
				sub = 0;
				peak[k]=i;
				peakC[k]=spidatas.des[i];
				//printf("init:%d--%d\n",peak[k],peakC[k]);
				k++;
			}

			sum100s = sum100;
			sum100	=	0;
		}
		if(k>2)//先判断一下峰数量
		{
			for(i=0;i<(k-1);i++){
				if(peakC[i] > peakC[i+1]){
					a=peakC[i];
				}
				else{
					a=peakC[i+1];
				}
			}
			//printf("a = %d",a);
			for(i=0;i<k;i++){
				//printf("text:peakC=%d,a/10=%d\n",peakC[i],(a/10));
				if(peakC[i] > (a/10)){
					peaks[m]=peak[i];
					peakCs[m]=peakC[i];
					//printf("end:%d--%d\n",peaks[m],peakCs[m]);
					m++;
				}
			}
			//半高宽和峰位比值
			//根据峰位的偏差量调整高压
		}
		else{
			//峰数量加压
		}

		sum100 	= 	0;
		sum100s	=	0;
		add 	= 	0;
		sub 	= 	0;
		k		=	0;
		m		=	0;

//	}

	return 0;
}
void adjust_win(void)
{
	int winA1ed;
	int winA2ed;
	int winB1ed;
	int winB2ed;
	int winC1ed;
	int winC2ed;
	int winE1ed;
	int winE2ed;

	winB1ed=(int)(((spisets.winB1-shared->b1)/shared->a1)*shared->a2+shared->b2);
	winB2ed=(int)(((spisets.winB2-shared->b1)/shared->a1)*shared->a2+shared->b2);

	winC1ed=(int)(((spisets.winC1-shared->b1)/shared->a1)*shared->a2+shared->b2);
	winC2ed=(int)(((spisets.winC2-shared->b1)/shared->a1)*shared->a2+shared->b2);

	winE1ed=(int)(((spisets.winE1-shared->b1)/shared->a1)*shared->a2+shared->b2);
	winE2ed=(int)(((spisets.winE2-shared->b1)/shared->a1)*shared->a2+shared->b2);

/*	winB1ed = winB1ed * ParaAdjust.Adjust_WinB_a + ParaAdjust.Adjust_WinB_b;
	winB2ed = winB2ed * ParaAdjust.Adjust_WinB_a + ParaAdjust.Adjust_WinB_b;
	winC1ed = winC1ed * ParaAdjust.Adjust_WinC_a + ParaAdjust.Adjust_WinC_b;
	winC2ed = winC2ed * ParaAdjust.Adjust_WinC_a + ParaAdjust.Adjust_WinC_b;
	winE1ed = winE1ed * ParaAdjust.Adjust_WinE_a + ParaAdjust.Adjust_WinE_b;
	winE2ed = winE2ed * ParaAdjust.Adjust_WinE_a + ParaAdjust.Adjust_WinE_b;
	printf("B_a=%f B_b=%f\n C_a=%f C_b=%f\n E_a=%f E_b=%f\n",ParaAdjust.Adjust_WinB_a,ParaAdjust.Adjust_WinB_b,
			ParaAdjust.Adjust_WinC_a,ParaAdjust.Adjust_WinC_b,ParaAdjust.Adjust_WinE_a,ParaAdjust.Adjust_WinE_b);*/
	printf("A1=%d A2=%d\n B1=%d B2=%d\n E1=%d E2=%d\n",winB1ed,winB2ed,winC1ed,winC2ed,winE1ed,winE2ed);

	spisets.A1	=	winB1ed;
	spisets.A2	=	winB2ed;
	spisets.B1	=	winC1ed;
	spisets.B2	=	winC2ed;
	spisets.E1	=	winE1ed;
	spisets.E2	=	winE2ed;

//	SetTH(0x01,winA1ed*2);
//	SetTH(0x02,winA2ed*2);

	SetTH(0x03,winB1ed*2);
	SetTH(0x04,winB2ed*2);

	SetTH(0x08,winC1ed*2);
	SetTH(0x0e,winC2ed*2);
//
	//SetTHS(0x1f,spisets.winD1*2);
	//SetTHS(0x21,spisets.winD2*2);

	SetTHS(0x23,winE1ed*2);
	SetTHS(0x25,winE2ed*2);

}
void errorCode(void)
{
	int i;
	float sum;
	float a;
	float b;
	for(i=200;i<7500;i++)//100道开始7500道结束
	{
		sum = es[i]+es[i+1];
		if(sum<=20.0)
		{
			if(es[i+2]>=30.0)
				es[i+2]=(es[i]+es[i+1])/2;
		}
/*		else if(sum>20 && sum <5000)
		{
			a = (float)(data_adj[i]+data_adj[i+1])/(2*(float)data_adj[i+2]);
			if(a<0.5 || a>1.5)
				data_adj[i+2]=(data_adj[i]+data_adj[i+1])/2;
		}*/
		else
		{
			a = es[i]+5*pow(es[i],0.5);//*2;
			b = es[i]-5*pow(es[i],0.5);//*0.2;
			if((es[i-1]<b||es[i-1]>a)&&(es[i+1]<b||es[i+1]>a))
			{
				es[i]=(es[i-1]+es[i+1])/2;
			}
			else if((es[i-1]<b||es[i-1]>a)&&(es[i+1]>b&&es[i+1]<a))
			{
				es[i-1]=(es[i]+es[i+1])/2;
			}
			else if((es[i-1]>b&&es[i-1]<a)&&(es[i+1]<b||es[i+1]>a))
			{
				es[i+1]=(es[i]+es[i-1])/2;
			}
			else
			{

			}

		}
	}
	for(i=0;i<8192;i++)
	{
		shared->S_es[i]=es[i];
	}
}

