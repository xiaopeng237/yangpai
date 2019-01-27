/*
 * data.h
 *
 *  Created on: 2016-4-8
 *      Author: vmuser
 */
#ifndef DATA_H
#define DATA_H

typedef struct
{

	float 	P_CPM_A;//绝压数值修正系数 数据库读取
	float 	P_CPM_B;//绝压数值修正系数 数据库读取
	float 	DP_CPM_A;//差压数值修正系数 数据库读取
	float 	DP_CPM_B;//差压数值修正系数 数据库读取
	float 	DP_CPM_C;//差压温度补偿 数据库读取
	float 	DP_CPM_D;//差压绝压补偿 数据库读取

	float	TF_PCPM_SC;//标况华式摄氏度 数据库读取
	float	P_CPM_SC;//标况压力 数据库读取
	float	F_CPM;//356/31比值 数据库读取
	float	D_CPM;//管直径，喉颈位置 数据库读取
	float	G31_CPM_A;
	float	G31_CPM_B;
	float	W31_CPM;//满水31吸收系数 数据库读取
	float	G356_CPM_A;
	float	G356_CPM_B;
	float	W356_CPM;//满水356吸收系数 数据库读取
	float	WD_CPM;//水密度LC 工况；数据库读取
	float	GD_CPM_SC;//气密度SC 标况；数据库读取
	float	b_CPM;//贝塔值 数据库读取
	float	C_CPM;//流出系数 数据库读取
	float	E_CPM;//流束膨胀系数 数据库读取
	//计算结果
	float	OVF_CPM;
	float	GVF_CPM;
	float	Qv_CPM;
	float	OMF_CPM;
	float	GMF_CPM;
	float	Qm_CPM;

}cpm;

typedef struct
{
	float	OVF_CPM_A;
	float	OVF_CPM_B;
	float	OMF_CPM_A;
	float	OMF_CPM_B;
	float	Qv_CPM_A;
	float	Qv_CPM_B;
	float	Qm_CPM_A;
	float	Qm_CPM_B;
	float	K31_Empty;
	float	CMP_ave;
	float	DP_th;
}corr;
typedef struct
{
	float	Adjust_WinB_a;
	float	Adjust_WinB_b;
	float	Adjust_WinC_a;
	float	Adjust_WinC_b;
	float	Adjust_WinE_a;
	float	Adjust_WinE_b;

}Adjust;

typedef struct
{
	int fd_RTC;
	int year;
	int month;
	int date;
	int hour;
	int minute;
	int second;
	long int timestamp;
	long int timestamps[600];
}datetime;

typedef struct
{
	float 	Measure_Tem;
	float 	Measure_Pre;
	float 	Measure_DP[200];
	int 	Win1[200];
	int		Win2[200];
	int		Win3[200];
	int		Win4[200];
	int		Win5[200];
	//计算中间量
	float	oilDensityLC;
	float	waterDensityLC;
	float	gasDensityLC;
	float 	Bo;
	float 	Bw;
	float 	Bg;
	float 	Z;
	float 	Rs;

}Cpar;

typedef struct
{
	int		fd_i2c;
	int		addr;
	float	ad1;
	float	ad2;
	float	ad3;
}i2cdata;

typedef struct
{
	int pch;
	int th;
	int winA1;
	int winA2;
	int winB1;
	int winB2;
	int winC1;
	int winC2;
	int winD1;
	int winD2;
	int winE1;
	int winE2;
	int st;
	int ct;
	int pth;
	int pmin;
	int pmax;
	int bl;
	int A1;
	int A2;
	int B1;
	int B2;
	int E1;
	int E2;
}spiset;

typedef struct
{
	int k31;
	int k81;
	int k160;
	int k302;
	int k356;
	int es;
	int esF;
	int pul;
	int pulF;
	int dpul[512];
	unsigned int des[8192];
	int fd_spi;
}spidata;

typedef struct
{
	float foil;
	float fgas;
	float fwater;
	float OMF;
	float GMF;
	float WMF;
	int 	winA;
	int		winB;
	int		winC;
	int    winD;
	int    winE;
	float foils[600];
	float fgass[600];
	float fwaters[600];
	float tem1s[600];
	float tem2s[600];
	float pres[600];
}flowR;

typedef struct
{
	//启动标志
	int flag_modbus;							//modbus，TCP/RTU选择
	int flag_oled;								//oled 是否显示
	int flag_es;								//能普数据 是否存储
	int flag_pulse;								//脉冲数据 是否存储
	int flag_5ms;								//5ms数据  是否存储
	int flag_MCA;								//是否生成.mca文件
	int flag_dog;								//是否启动看门狗
	int flag_log;

	//运行标志
	//int

	//运行参数
	//int arm_hv;
	//int arm_vbb;

}flag;
/*
 * 流量运算参数
 */
typedef struct
{
	float oilExpansionFactor;
	float waterExpansionFactor;
	float localPressure;
	float TempertureSC;
	float oilDensitySC;
	float waterDensitySC;
	float gasDensitySC;
	float Gr;
	float nMC;
	float nMn;
	float nMC1;
	float nZn;
	float nPb;//
	float referenceTempertaureSC;

}PVT_BlackOil;

typedef struct
{
	float beatO;
	float beatW;
	float P0;
	float T0;
	float oilDensitySC;
	float gasDensitySC;
	float waterDensitySC;
	float ntime;

}PVT_Normal;

typedef struct
{
	float Energy1_Empty;
	float Energy1_Oil;
	float Energy1_Gas;
	float Energy1_Water;
	float Energy2_Empty;
	float Energy2_Oil;
	float Energy2_Gas;
	float Energy2_Water;
	float Energy5_Empty;
	float Energy5_Oil;
	float Energy5_Gas;
	float Energy5_Water;
	float f1;
	float f2;
	float C;
	float epsilon;
	float Diameter;
	float beta;

}PFC_Three;

typedef struct
{
	float Qm_Oil;
	float Qm_Gas;
	float Qm_Water;

	float OMF;
	float GMF;
	float WMF;
	float Qm_Oils[200];
	float Qm_Gass[200];
	float Qm_Waters[200];
	float oilDensityLC;
	float waterDensityLC;
	float gasDensityLC;

}Flow_result;

typedef struct
{
	unsigned long int spi_en_sum;
	float emptyhighCount;//空管计数
	float LlinearatTenuation;
	float HlinearatTenuation;
	float KlinearatTenuation;
	float emptylowCount;//空管计数
	float emptykCount;//空管计数
	float absorptionDistance;// 厚度
	float math_flag;//计算标志位
	int PCH;
	int TH;
	int TH0;
	int TH1;
	int TH2;
	int TH3;
	int TH4;
	int pulse_thr;
	int pulse_min;
	int pulse_max;
	float k31;
	float k81;
	float escaping;
	int pulse_mode;
	int ct;
	int st;
	int fd_spi;
	int fd_i2c;

	int log_fd;
	int peak_flag;
	int peak_flags;///闻风标志位
	int peak_flagss;
	int peak_num;
	int write_flag;
	int write_flags;
	int high_v;
	int low_v;
	int addr;
	int addrs;
	int q;
	int fd_creat;
	int peak_one;
	int peak_three;
	float adc[4];
	float cv_ratio;
	unsigned char a[1200];
	unsigned char b[1200];
	unsigned char time[12];
	unsigned char spidata[60];
	unsigned char spidata_en[1024];
	unsigned char pulse_data[1024];
	unsigned char hartdata[30];
	unsigned char modbusdata[512];

	int mode_select;
	int mode_time;

	int	NUM;
	float	liquidFlowLC;
	float	oilFlowLC;
	float	waterFlowLC;
	float	gasFlowLC;
	float	liquidFlowSC;
	float	oilFlowSC;
	float	waterFlowSC;
	float	gasFlowSC;
	float	waterLiquidRatio;
	float   gasVoidFraction;
	float 	temperature;
	float	pressure;
	float	diffPressure;
	float	flowMixDensity;
	float	highEnergyCount;
	float	lowEnergyCount;
	float   dualGammaGVF;
	float	dualGammaWLR;
	float	dualGammaLHU;
	float	alarmCode;
	float	mixViscosity;
	float	Red;
	float   nDischargeCoefficient;
	float	nSlipRatio;
	float	oilDensityLC;
	float	waterDensityLC;
	float	gasDensityLC;
	float	Bo;
	float	Bw;
	float	Bg;
	float	Rs;
	float	Z;
}hander_result;


int i2cR(hander_result * p_result);
int i2cW(hander_result * p_result);
int i2c_Vadjust(float ratio,hander_result * p_result);
int adc(hander_result * p_result);
int MB_serical(unsigned char id,int iFd,hander_result * p_result);
int MB_TCP(unsigned char id, hander_result * p_result);
void MB_data(hander_result * p_result);
void MB_datas(hander_result * pt);

void transfer(int fd1,hander_result * pt);
int Peak_adj(int fd1,hander_result * pt);

int cal_spe_once(hander_result * pt);
void change_threshold(int dirft_from_flag,hander_result * pt);
void reset_threshold(hander_result * pt);
int spi_oper(hander_result * pt);
int en_check(hander_result * pt);


int SQLite (hander_result * p_result);

int Hart_burst(int iFd,hander_result * p_result);
int Creat_xml(hander_result* p_result);
void datawrite (hander_result * pt);
void datawrites(hander_result * p_result);
int get_parameter(hander_result * pt);

int open_log(hander_result * pt);
int close_log(hander_result * pt);
int w_log(hander_result * pt);
int en_data_w(hander_result *pt);
int en_data_ws(hander_result *pt);
int realtime_w(hander_result *pt);
void peak_pw(hander_result *pt);



void setDefaultThs(int fd1,hander_result * pt);
int spi_set(hander_result * ptr);
int spi_sets(hander_result * ptr);
int spi_datapro(hander_result * ptr);




#endif
