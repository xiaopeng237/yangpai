/*
 ============================================================================
 Name        : yangpai.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <pthread.h>
#include "readpara.h"
#include "dataacq.h"
#include "data.h"
#include "i2cwork.h"
#include "spi.h"
#include <unistd.h>
#include <sys/shm.h>
#include "shmdata.h"

MYSQL 			mysql_conn;
datetime 		m_time;
i2cdata 		i2c;
Cpar			cpar;
spiset 			spisets;
spidata 		spidatas;
flowR			flowresult;
flag			flags;
hander_result * ptr;
int 			win_C[200][6];//计算用的二维数组
int fd_dog;

PVT_BlackOil	PVT_BlackOils;
PVT_Normal		PVT_Normals;
PFC_Three		PFC_Threes;
Flow_result		Flow_results;
corr			corrs;
cpm				cpms;
//脱离线程所需要的变量
pthread_attr_t	thread_attr;
pthread_attr_t 	thread_attr1;
pthread_attr_t 	thread_attr2;
pthread_attr_t 	thread_attr3;
pthread_t 		a_thread;
pthread_t 		a_thread1;
pthread_t 		a_thread2;
pthread_t 		a_thread3;

struct shared_use_st *shared;//指向shm
int sys_init(void)
{
	//创建脱离线程属性
	int res;
	res = pthread_attr_init(&thread_attr);
	if(res !=0){
		perror("failed 1");
		exit(EXIT_FAILURE);
	}
	//将线程属性设置为脱离线程
	res = pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED);
	if(res != 0){
		perror("failed 2");
		exit(EXIT_FAILURE);
	}

	res = pthread_attr_init(&thread_attr1);
	if(res !=0){
		perror("failed 1");
		exit(EXIT_FAILURE);
						}
	//将线程属性设置为脱离线程
	res = pthread_attr_setdetachstate(&thread_attr1,PTHREAD_CREATE_DETACHED);
	if(res != 0){
		perror("failed 2");
		exit(EXIT_FAILURE);
	}

	res = pthread_attr_init(&thread_attr2);
	if(res !=0){
		perror("failed 1");
		exit(EXIT_FAILURE);
	}
	//将线程属性设置为脱离线程
	res = pthread_attr_setdetachstate(&thread_attr2,PTHREAD_CREATE_DETACHED);
	if(res != 0){
		perror("failed 2");
		exit(EXIT_FAILURE);
	}

	res = pthread_attr_init(&thread_attr3);
	if(res !=0){
		perror("failed 1");
		exit(EXIT_FAILURE);
	}
	//将线程属性设置为脱离线程
	res = pthread_attr_setdetachstate(&thread_attr3,PTHREAD_CREATE_DETACHED);
	if(res != 0){
		perror("failed 2");
		exit(EXIT_FAILURE);
	}
	//i2c初始化
	i2c.fd_i2c = i2c_function();
	//高压模块初始化给一个高压
	i2c.addr = 0x55;
	i2c_CV (i2c.addr);//没有用
	//spi初始化
	spidatas.fd_spi = spi();
	setDefaultTh();
	//1115初始化 1115初始化要在i2c初始化的后面
	i2c_readADC_set();//没有用
	return 0;
}
int sys_end(void)
{
	mysql_close(&mysql_conn);
	close(m_time.fd_RTC);
	return 0;
}
int main(int argc, char** argv) {
	sleep(15);

//创建共享内存
//*********************************
//	int running = 1;//程序是否继续运行的标志
	void *shm = NULL;//分配的共享内存的原始首地址
	int shmid;//共享内存标识符
//创建共享内存
	shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666|IPC_CREAT);//01000
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
//将共享内存连接到当前进程的地址空间
	shm = shmat(shmid, 0, 0);
	if(shm == (void*)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	printf("\nMemory attached at %X\n", (int)shm);
//设置共享内存
	shared = (struct shared_use_st*)shm;

//初始化mysql结构
//*********************************
	if(mysql_init(&mysql_conn) == NULL)
			printf("Initialization Failed.\n");
	if(mysql_real_connect(&mysql_conn, "localhost", "root", "root", NULL, MYSQL_PORT, NULL, 0) == NULL)//链接一个mysql数据库
			printf("Connection Failed!\n");
//读取各种参数
//*********************************
	shared->S_F356=0;
	shared->flag_hv=0;
	shared->Qm_SUM=0;
	shared->Qml_CPM = 0;
	shared->Qmg_CPM = 0;
	readpara();
	//write_mysql_data();
/*
 * 获取程序pid
 */
	shared->pid = getpid();
	printf("pid=%d\n",shared->pid);
/*
 * 设置高压
 */
	shared->S_HIVf=1;
	shared->S_VBBf=1;
	char sys[30];
	if(shared->S_VBB < 1500)
	{
		sprintf(sys,"/mnt/i2c_2631vbb %d",shared->S_VBB);
		system(sys);
	}
	if(shared->S_HIV < 3000)
	{
		sprintf(sys,"/mnt/i2c_2631hv %d",shared->S_HIV);
		system(sys);
	}


//****************************************
	puts("!!!yangpai!!!");
	system("killall -9 freemodbus_rtu");
	system("killall -9 freemodbus_tcp");
	system("killall -9 i2c_bus");
//	system("/mnt/ad1115 &");//差压数据
//	system("/mnt/Tem_R1115 &");//内外温度数据；压力数据
	system("/mnt/i2c_bus &");
	printf("=======%d\n",flags.flag_log);
	if(flags.flag_oled!=0){
		//system("/mnt/i2c_oled &");//oled显示
	}
	if(flags.flag_modbus!=0){
		printf("MODBUS RTU working\n");
		system(" /mnt/freemodbus_rtu e &");//modbus功能
	}
	else{
		printf("MODBUS TCP working\n");
		system(" /mnt/freemodbus_tcp e &");//modbus功能
	}

	if(flags.flag_dog!=0){

	    fd_dog = open("/dev/watchdog", O_WRONLY);

	    if (fd_dog == -1) {
		fprintf(stderr, "Watchdog device not enabled.\n");
		fflush(stderr);
		exit(-1);
	    }
		fprintf(stderr, "Watchdog Ticking Away!\n");
		fflush(stderr);
	}
	sleep(5);
//系统初始化设置高压创建脱离线程
//*********************************
	sys_init();
	spectral_data_w();
//	FPGA_data_w();

//获取各种数据
//*********************************
	dataacq();
//	intersignal();
//	interrupt();
//***********************************
//把共享内存从当前进程中分离
	if(shmdt(shm) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
//删除共享内存
	if(shmctl(shmid, IPC_RMID, 0) == -1)//0
	{
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);

//**************************

	return EXIT_SUCCESS;
}
