/*
 * shmdata.h
 *
 *  Created on: 2016-5-3
 *      Author: vmuser
 */


#ifndef _SHMDATA_H_HEADER
#define _SHMDATA_H_HEADER

#define TEXT_SZ 2048

struct shared_use_st
{
	int written;//用于差压读取
	char text[TEXT_SZ];//记录写入和读取的文本
	unsigned short modbusdata[1024];
	float ad1115[200][3];
	float tem1;
	float tem2;
	int S_HV;//高压
	int S_VBB;
	float S_HV_C;//高压调节系数
	int S_PCH31;//三个峰的预设位置
	int S_PCH81;
	int S_PCH356;
	float change31;//三个峰的偏移系数，根据他们调整WIN
	float change81;
	float change356;
	int F_WIN_J;//卷积控制位
	int S_F31;//实际峰位
	int S_F81;
	int S_F356;
	int CON_A1;
	int CON_A2;
	int CON_B1;
	int CON_B2;
	int CON_C1;
	int CON_C2;
	int CON_D1;
	int CON_D2;
	int CON_E1;
	int CON_E2;
	unsigned int S_es[8192];//能普
	float DEV_T1;//设备板温度
	float DEV_T2;
	float DEV_T3;
	int S_HIV;//高压
	int pid;
	int S_VBBf;//调节偏置电压位
	int S_HIVf;//调压命令位
	float Pre_a;
	float Pre_b;
	float DP_a;
	float DP_b;
	float PreT_a;
	float PreT_b;
	int HIV_en;//是否自动调压
	int F_WIN_en;//是否自动卷机
	int OLED_en;//是否oled显示
	int W_Es;
	int W_5ms;
	int W_log;
	int W_pulse;
	unsigned int th_sum;
	int			flag_hv;
	float a1;
	float b1;
	float a2;
	float b2;
	int adjust_HV;
	float	OMF_CPM;
	float	GMF_CPM;
	float	Qm_CPM;
	float	Qm_SUM;
	float	Gm_SUM;
	float	DataP;//绝压
	float	DataDP;//差压

	int		hour;
	int		minute;
	int		second;
	float	Qml_CPM;
	float	Qmg_CPM;

};

#endif


