/*
 * flow_calculation.c
 *
 *  Created on: 2016-4-11
 *      Author: vmuser
 */
#include "flow_calculation.h"
#include "spmath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "data.h"
//#include "SQLite.h"
//#include "datawrite.h"
//#include "Creat_xml.h"

//#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
//#include<sys/ioctl.h>

packageHead  		m_packageHead;
backgroundInfo 		m_backgroundInfo;
publicParameter 	m_publicParameter;
nVenturi			m_nVenturi;
modelParameter		m_modelParameter;
extern hander_result * ptr;

unsigned char wellID;
measureResult m_measureResult;
measureResult t_measureResult;//temp
measureResult s_measureResult;
unsigned char code_Basid;

nADChannel m_nADChannel[8];

calParameter highCalParameter;
calParameter lowCalParameter;

nMVT 		m_nMVT;
nMVT        s_nMVT;
nMVT        t_nMVT;//temp

dualGamma 	m_dualGamma;
dualGamma   t_dualGamma;
dualGamma   s_dualGamma;

resultPVT	t_resultPVT;
resultPVT	m_resultPVT;
resultPVT	s_resultPVT;
float paraPVTSim[90]={0};

unsigned    char gamaCnt;
unsigned    char logCnt;
unsigned    char calCnt;
unsigned 	char m_Start ;

void HMPrInit(void)
{
   	m_Start =0;
   	gamaCnt =0;
   	logCnt =0;
   	m_resultPVT.Bo=1.0;
   	m_resultPVT.Bg=1.0;
   	m_resultPVT.Bw=1.0;
   	m_resultPVT.Z =1.0;
   	m_resultPVT.Rs=1.0;

    loadConfig();

}
int loadConfig(void)
{
	loadPara1();
	loadPara2();
	loadPara3();
	loadPara4();
	loadCali();
//	printf("5235!!!!!!!!!!!!1");
	return 0;
}
//可以读取30个参数
void loadPara1(void)
{
	int fd4;
	int i,ic;
	int k = 0;
	float f[30];
	char str[300];
	char para[30][10];
	fd4 = open("/mnt/para1.csv", O_RDWR | O_CREAT);

	ic = read(fd4,str,sizeof(str));
	for(i=0;i<ic;i++)
	{
		if(str[i]==',')
		{
			lseek(fd4,i+1,SEEK_SET);
			read(fd4,para[k],10);
			f[k] = atof(para[k]);
			printf("1:%f\n",f[k]);
			k++;
		}
	}
		m_publicParameter.oilExpansionFactor = f[0];
//		printf("%f=====",m_publicParameter.oilExpansionFactor);
		m_publicParameter.waterExpansionFactor=f[1];
		m_publicParameter.oilDensitySC=f[2];
		m_publicParameter.waterDensitySC=f[3];
		m_publicParameter.gasDensitySC=f[4];

		m_publicParameter.localBarometricPressure=f[5];
		m_publicParameter.referenceTemperatureSC=f[6];
		m_publicParameter.Gr=f[7];

		m_nADChannel[0].upLimitValue = f[8];
		m_nADChannel[0].downLimitValue = f[9];
		m_nADChannel[1].upLimitValue = f[10];
		m_nADChannel[1].downLimitValue = f[11];
		m_nADChannel[2].upLimitValue = f[12];
		m_nADChannel[2].downLimitValue = f[13];
		m_nADChannel[3].upLimitValue = f[14];
		m_nADChannel[3].downLimitValue = f[15];
		m_nVenturi.venturi_C = f[16];
		m_nVenturi.venturi_E = f[17];
		m_nVenturi.venturi_d = f[18];
		m_nADChannel[4].upLimitValue = f[19];//80wan
		m_nADChannel[4].downLimitValue = f[20];
		m_nADChannel[5].upLimitValue = f[21];
		m_nADChannel[5].downLimitValue = f[22];
		m_nADChannel[6].upLimitValue = f[23];
		m_nADChannel[6].downLimitValue = f[24];
		m_nADChannel[7].upLimitValue = f[25];
		m_nADChannel[7].downLimitValue = f[26];


	close (fd4);
}
void loadPara2(void)
{
	int fd4;
	int i,ic;
	int k = 0;
	float f[30];
	char str[300];
	char para[30][10];
	fd4 = open("/mnt/para2.csv", O_RDWR | O_CREAT);

	ic = read(fd4,str,sizeof(str));
	for(i=0;i<ic;i++)
	{
		if(str[i]==',')
		{
			lseek(fd4,i+1,SEEK_SET);
			read(fd4,para[k],10);
			f[k] = atof(para[k]);
			printf("2:%f\n",f[k]);
			k++;
		}
	}
		m_modelParameter.oilShrinkageFactor = f[0];
		m_modelParameter.zFactor = f[1];
		m_modelParameter.nGor = f[2];

		m_modelParameter.diffPressureThreshod = f[3];
		m_nVenturi.venturi_H = f[4];

		m_modelParameter.nPVTModelSelect = f[5];
		m_modelParameter.nC1 = f[6];
		m_modelParameter.nC2 = f[7];
		m_modelParameter.nC3 = f[8];
		m_modelParameter.nC4 = f[9];
		m_modelParameter.nC5 = f[10];
		m_modelParameter.nC6 = f[11];
		m_modelParameter.nZn = f[12];
		m_modelParameter.nMn = f[13];
		m_modelParameter.nMC = f[14];
		m_modelParameter.nMC1 = f[15];
		m_modelParameter.nPb = f[16];

		m_nVenturi.venturi_D = f[17];

		m_modelParameter.mixVisSel = f[18];
		m_modelParameter.oilVisSel = f[19];

	close (fd4);
}
void loadPara3(void)
{
	int fd4;
	int i,ic;
	int k = 0;
	float f[30];
	char str[300];
	char para[30][10];
	fd4 = open("/mnt/para3.csv", O_RDWR | O_CREAT);

	ic = read(fd4,str,sizeof(str));
	for(i=0;i<ic;i++)
	{
		if(str[i]==',')
		{
			lseek(fd4,i+1,SEEK_SET);
			read(fd4,para[k],10);
			f[k] = atof(para[k]);
			printf("3:%f\n",f[k]);
			k++;
		}
	}
		m_modelParameter.oilMolecularWeight = f[0];
		m_modelParameter.oilVisA = f[1];
		m_modelParameter.oilVisB = f[2];
		m_modelParameter.oilVisC = f[3];
		m_modelParameter.slipA = f[4];
		m_modelParameter.slipB = f[5];
		m_modelParameter.slipC = f[6];
		m_modelParameter.Reserve1 = f[7];
		m_modelParameter.Reserve2 = f[8];
		m_modelParameter.Reserve3 = f[9];
		m_modelParameter.Reserve4 = f[10];
		m_modelParameter.Reserve5 = f[11];
		m_modelParameter.Reserve6 = f[12];
		m_modelParameter.Reserve7 = f[13];
		m_modelParameter.Reserve8 = f[14];
		m_modelParameter.Reserve9 = f[15];
		m_modelParameter.Reserve10 = f[16];

	close (fd4);
}
void loadPara4(void)
{
	int fd4;
	int i,ic;
	int k = 0;
	float f[50];
	char str[500];
	char para[50][10];
	fd4 = open("/mnt/para4.csv", O_RDWR | O_CREAT);

	ic = read(fd4,str,sizeof(str));
	for(i=0;i<ic;i++)
	{
		if(str[i]==',')
		{
			lseek(fd4,i+1,SEEK_SET);
			read(fd4,para[k],10);
			f[k] = atof(para[k]);
			printf("%f\n",f[k]);
			k++;
		}
	}
		paraPVTSim[11] = f[0];
		paraPVTSim[12] = f[1];
		paraPVTSim[13] = f[2];
		paraPVTSim[14] = f[3];
		paraPVTSim[15] = f[4];
		paraPVTSim[16] = f[5];
		paraPVTSim[21] = f[6];
		paraPVTSim[22] = f[7];
		paraPVTSim[23] = f[8];
		paraPVTSim[24] = f[9];
		paraPVTSim[25] = f[10];
		paraPVTSim[26] = f[11];
		paraPVTSim[31] = f[12];
		paraPVTSim[32] = f[13];
		paraPVTSim[33] = f[14];
		paraPVTSim[34] = f[15];
		paraPVTSim[35] = f[16];
		paraPVTSim[36] = f[17];
		paraPVTSim[41] = f[18];
		paraPVTSim[42] = f[19];
		paraPVTSim[43] = f[20];
		paraPVTSim[44] = f[21];
		paraPVTSim[45] = f[22];
		paraPVTSim[46] = f[23];
		paraPVTSim[51] = f[24];
		paraPVTSim[52] = f[25];
		paraPVTSim[53] = f[26];
		paraPVTSim[54] = f[27];
		paraPVTSim[55] = f[28];
		paraPVTSim[56] = f[29];
		paraPVTSim[61] = f[30];
		paraPVTSim[62] = f[31];
		paraPVTSim[63] = f[32];
		paraPVTSim[64] = f[33];
		paraPVTSim[65] = f[34];
		paraPVTSim[66] = f[35];
		paraPVTSim[71] = f[36];
		paraPVTSim[72] = f[37];
		paraPVTSim[73] = f[38];
		paraPVTSim[74] = f[39];
		paraPVTSim[75] = f[40];
		paraPVTSim[76] = f[41];
		paraPVTSim[81] = f[42];
		paraPVTSim[82] = f[43];
		paraPVTSim[83] = f[44];
		paraPVTSim[84] = f[45];
		paraPVTSim[85] = f[46];
		paraPVTSim[86] = f[47];

	close (fd4);
}
void loadCali(void)
{
	int fd4;
	int i,ic;
	int k = 0;
	float f[30];
	char str[300];
	char para[30][10];
	fd4 = open("/mnt/cali.csv", O_RDWR | O_CREAT);
//	printf("5235!!!!!!!!!!!!1");
	ic = read(fd4,str,sizeof(str));
//	printf("%d",ic);
	for(i=0;i<ic;i++)
	{
		if(str[i]==',')
		{
			lseek(fd4,i+1,SEEK_SET);
			read(fd4,para[k],10);
			f[k] = atof(para[k]);
			printf("C:%f\n",f[k]);
			k++;

		}

	}
		highCalParameter.emptyPipeCount = f[0];
		highCalParameter.gasAbsorptionCoefficient = f[1];
		highCalParameter.waterAbsorptionCoefficient = f[2];
		highCalParameter.oilAbsorptionCoefficient = f[3];
		highCalParameter.absorptionDistance = f[4];
		highCalParameter.calTemperature = f[5];
		lowCalParameter.emptyPipeCount = f[6];
		lowCalParameter.gasAbsorptionCoefficient = f[7];
		lowCalParameter.waterAbsorptionCoefficient = f[8];
		lowCalParameter.oilAbsorptionCoefficient = f[9];
		lowCalParameter.absorptionDistance = highCalParameter.absorptionDistance;
		lowCalParameter.calTemperature = highCalParameter.calTemperature;
/*		if(m_modelParameter.Reserve4 ==1)//如果是钡源，则计算衰减
			{
				getFileTime("0:");//获取时间并计算源衰减
			}*///以后再考虑

	close (fd4);
}
void getResult(float L,float H)
{
//	char *p;
	flowcalculation(L,H);
	calCnt++;

//显示用的
	if(calCnt==1)
	{
//	   FlashRead(0xffc00634, p, 1);

	   t_nMVT.temperature 	= s_nMVT.	temperature/60;

	   t_nMVT.pressure 		= s_nMVT.	pressure/60;
	   t_nMVT.diffPressure 	=s_nMVT.	diffPressure/60;

	   t_resultPVT.Bo      	= s_resultPVT.Bo/60;
	   t_resultPVT.Bw      	= s_resultPVT.Bw/60;
	   t_resultPVT.Bg      	= s_resultPVT.Bg/60;
	   t_resultPVT.Z      	= s_resultPVT.Z/60;
	   t_resultPVT.Rs      	= s_resultPVT.Rs/60;

	   t_measureResult.liquidFlowLC	= s_measureResult.liquidFlowLC/60;
	   t_measureResult.oilFlowLC	= s_measureResult.oilFlowLC/60;
	   t_measureResult.waterFlowLC	= s_measureResult.waterFlowLC/60;
	   t_measureResult.gasFlowLC	= s_measureResult.gasFlowLC/60;
	   t_measureResult.liquidFlowSC	= s_measureResult.liquidFlowSC/60;
	   t_measureResult.oilFlowSC	= s_measureResult.oilFlowSC/60;
	   t_measureResult.waterFlowSC	= s_measureResult.waterFlowSC/60;
	   t_measureResult.gasFlowSC	= s_measureResult.gasFlowSC/60;
	   t_measureResult.dualGammaWLR	= s_measureResult.dualGammaWLR/60;
	   t_measureResult.dualGammaGVF	= s_measureResult.dualGammaGVF/60;
	   t_measureResult.dualGammaLHU	= s_measureResult.dualGammaLHU/60;


	   t_measureResult.flowMixDensity	= s_measureResult.flowMixDensity/60;
	   t_measureResult.mixViscosity	= s_measureResult.mixViscosity/60;
	   t_measureResult.Red	= s_measureResult.Red/60;
	   t_measureResult.nDischargeCoefficient	= s_measureResult.nDischargeCoefficient/60;
	   t_measureResult.nSlipRatio	= s_measureResult.nSlipRatio/60;

	   t_measureResult.oilDensityLC	= s_measureResult.oilDensityLC/60;
	   t_measureResult.waterDensityLC	= s_measureResult.waterDensityLC/60;
	   t_measureResult.gasDensityLC	= s_measureResult.gasDensityLC/60;

	   t_dualGamma.highEnergyCount =  s_dualGamma.highEnergyCount;
	   t_dualGamma.lowEnergyCount =  s_dualGamma.lowEnergyCount;
	}

	else
	{

	   t_nMVT.temperature 	= 	 s_nMVT.temperature /60	+	t_nMVT.temperature ;


	   t_nMVT.pressure 		=    s_nMVT.pressure/60		+	t_nMVT.pressure;
	   t_nMVT.diffPressure 	=	 s_nMVT.diffPressure/60	+	t_nMVT.diffPressure;

	   t_resultPVT.Bo      	= s_resultPVT.Bo/60 + t_resultPVT.Bo;
	   t_resultPVT.Bw      	= s_resultPVT.Bw/60 + t_resultPVT.Bw;
	   t_resultPVT.Bg      	= s_resultPVT.Bg/60 + t_resultPVT.Bg;

	   t_resultPVT.Z      	= s_resultPVT.Z /60 + t_resultPVT.Z;
	   t_resultPVT.Rs      	= s_resultPVT.Rs/60 + t_resultPVT.Rs;

	   t_measureResult.liquidFlowLC	= t_measureResult.liquidFlowLC +  s_measureResult.liquidFlowLC/60;
       t_measureResult.oilFlowLC	= t_measureResult.oilFlowLC +  s_measureResult.oilFlowLC/60;
       t_measureResult.waterFlowLC	= t_measureResult.waterFlowLC +  s_measureResult.waterFlowLC/60;
       t_measureResult.gasFlowLC	= t_measureResult.gasFlowLC +  s_measureResult.gasFlowLC/60;
       t_measureResult.liquidFlowSC	= t_measureResult.liquidFlowSC +  s_measureResult.liquidFlowSC/60;
       t_measureResult.oilFlowSC	= t_measureResult.oilFlowSC +  s_measureResult.oilFlowSC/60;
       t_measureResult.waterFlowSC	= t_measureResult.waterFlowSC +  s_measureResult.waterFlowSC/60;
       t_measureResult.gasFlowSC	= t_measureResult.gasFlowSC +  s_measureResult.gasFlowSC/60;
       t_measureResult.dualGammaWLR	= t_measureResult.dualGammaWLR +  s_measureResult.dualGammaWLR/60;
       t_measureResult.dualGammaGVF	= t_measureResult.dualGammaGVF +  s_measureResult.dualGammaGVF/60;
       t_measureResult.dualGammaLHU	= t_measureResult.dualGammaLHU +  s_measureResult.dualGammaLHU/60;

	   t_measureResult.flowMixDensity	= t_measureResult.flowMixDensity +  s_measureResult.flowMixDensity/60;
       t_measureResult.mixViscosity	= t_measureResult.mixViscosity +  s_measureResult.mixViscosity/60;
       t_measureResult.Red	= t_measureResult.Red +  s_measureResult.Red/60;
       t_measureResult.nDischargeCoefficient	= t_measureResult.nDischargeCoefficient +  s_measureResult.nDischargeCoefficient/60;
       t_measureResult.nSlipRatio	= t_measureResult.nSlipRatio +  s_measureResult.nSlipRatio/60;

       t_measureResult.oilDensityLC	= t_measureResult.oilDensityLC+s_measureResult.oilDensityLC/60;
	   t_measureResult.waterDensityLC=t_measureResult.waterDensityLC+ s_measureResult.waterDensityLC/60;
	   t_measureResult.gasDensityLC	=t_measureResult.gasDensityLC+ s_measureResult.gasDensityLC/60;

       t_dualGamma.highEnergyCount += s_dualGamma.highEnergyCount;
       t_dualGamma.lowEnergyCount += s_dualGamma.lowEnergyCount;


      if( calCnt == 60)//累加60次
       	{
       	   calCnt =0;

       	   m_nMVT.temperature = t_nMVT.	temperature;
       //			printf("calCnt = %d\n",calCnt);
       	   m_nMVT.pressure =    t_nMVT.	pressure;
       	   m_nMVT.diffPressure =t_nMVT.	diffPressure;
       	   t_nMVT.temperature = 0.0;
       	   t_nMVT.pressure =    0.0;
       	   t_nMVT.diffPressure =0.0;

       	   ptr->temperature =  m_nMVT.temperature;

       	   if(m_nMVT.pressure>0)
       	   ptr->pressure =    m_nMVT.pressure;
       	   else
       	   ptr->pressure = 0.0;
       	   if(m_nMVT.diffPressure>0)
       	   ptr->diffPressure =m_nMVT.diffPressure;
       	   else
       	   ptr->diffPressure =0.0;

              s_nMVT.temperature =0;//dqz
       	   s_nMVT.pressure = 0;
       	   s_nMVT.diffPressure =0;

       	   ptr->Bo      	= t_resultPVT.Bo;
       	   ptr->Bw      	= t_resultPVT.Bw;
       	   ptr->Bg      	= t_resultPVT.Bg;

       	   ptr->Z      	= t_resultPVT.Z;
       	   ptr->Rs      	= t_resultPVT.Rs;

       	   t_resultPVT.Bo      	= 0.0;
       	   t_resultPVT.Bw      	= 0.0;
       	   t_resultPVT.Bg      	= 0.0;
       	   t_resultPVT.Z      	= 0.0;
       	   t_resultPVT.Rs      	= 0.0;


       	   ptr->liquidFlowLC	= t_measureResult.liquidFlowLC;
       	   t_measureResult.liquidFlowLC =0.0;
       	   ptr->oilFlowLC	= t_measureResult.oilFlowLC;
       	   t_measureResult.oilFlowLC =0.0;
       	   ptr->waterFlowLC	= t_measureResult.waterFlowLC;
       	   t_measureResult.waterFlowLC =0.0;
       	   ptr->gasFlowLC	= t_measureResult.gasFlowLC;
       	   t_measureResult.gasFlowLC =0.0;

       	   ptr->liquidFlowSC	= t_measureResult.liquidFlowSC;
       	   t_measureResult.liquidFlowSC =0.0;
       	   ptr->oilFlowSC	= t_measureResult.oilFlowSC;
       	   t_measureResult.oilFlowSC =0.0;
       	   ptr->waterFlowSC	= t_measureResult.waterFlowSC;
       	   t_measureResult.waterFlowSC =0.0;
       	   ptr->gasFlowSC	= t_measureResult.gasFlowSC;
       	   t_measureResult.gasFlowSC =0.0;

       	   ptr->dualGammaWLR	= t_measureResult.dualGammaLHU/(1-t_measureResult.dualGammaGVF);
       	   t_measureResult.dualGammaWLR =0.0;
       	   ptr->dualGammaGVF	= t_measureResult.dualGammaGVF;
       	   t_measureResult.dualGammaGVF =0.0;
       	   ptr->dualGammaLHU	= t_measureResult.dualGammaLHU;
       	   t_measureResult.dualGammaLHU =0.0;
       //	   printf("Mgvf is %f,MLHU is %f,WLR is %f\n",m_measureResult.dualGammaGVF,m_measureResult.dualGammaLHU,m_measureResult.dualGammaWLR);
       	   ptr->flowMixDensity	= t_measureResult.flowMixDensity;
       	   t_measureResult.flowMixDensity =0.0;
       	   ptr->mixViscosity	= t_measureResult.mixViscosity;
       	   t_measureResult.mixViscosity =0.0;
       	   ptr->Red	= t_measureResult.Red;
       	   t_measureResult.Red =0.0;
       	   ptr->nDischargeCoefficient	= t_measureResult.nDischargeCoefficient;
       	   t_measureResult.nDischargeCoefficient =0.0;
       	   ptr->nSlipRatio	= t_measureResult.nSlipRatio;
       	   t_measureResult.nSlipRatio =0.0;

       	   ptr->oilDensityLC	= t_measureResult.oilDensityLC;
       	   t_measureResult.oilDensityLC =0.0;
       	   ptr->waterDensityLC	= t_measureResult.waterDensityLC;
       	   t_measureResult.waterDensityLC =0.0;
       	   ptr->gasDensityLC	= t_measureResult.gasDensityLC;
       	   t_measureResult.gasDensityLC =0.0;

       	   ptr->highEnergyCount	=t_dualGamma.highEnergyCount;
       	   t_dualGamma.highEnergyCount =0.0;
       	   ptr->lowEnergyCount	=t_dualGamma.lowEnergyCount;
       	   t_dualGamma.lowEnergyCount =0.0;

       	   ptr->waterLiquidRatio = m_measureResult.dualGammaWLR;
       	   ptr->gasVoidFraction     = m_measureResult.dualGammaGVF;

       	   ptr->highEnergyCount = 	m_dualGamma.highEnergyCount;
       	   ptr->lowEnergyCount = 	m_dualGamma.lowEnergyCount;
       	   //网页显示存储and数据存储


    // 		   datawrite(ptr);//100ms
   //    		en_data_w(ptr);//26ms
  //   		   SQLite(ptr);//5ms
  //     		   realtime_w(ptr);


       	}

    }

}
//标定程序
void demarcate()
{
	switch(ptr->mode_select){
		case 1:
				emptycon();
				break;
		case 2:
				oilcon();
				break;
		case 3:
				gascon();
				break;
		case 4:
			watercon();
				break;
		default :
				break;
	}
}
void emptycon()
{
	static long int Lempty_c;
	static long int Hempty_c;
	static long int Kempty_c;
	static long int time_c;
	float ave_L;
	float ave_H;
	float ave_K;
	if(time_c < ptr->mode_time*60000/ptr->st){
		Lempty_c = ptr->spidata[0]*256+ptr->spidata[1] + Lempty_c;
		Hempty_c = ptr->spidata[2]*256+ptr->spidata[3] + Hempty_c;
		Kempty_c = ptr->spidata[4]*256+ptr->spidata[5] + Kempty_c;

		time_c++;
	}
	else{
		Lempty_c = Lempty_c - Kempty_c*ptr->k31 - ptr->escaping*Hempty_c;
		Hempty_c = Hempty_c - Kempty_c*ptr->k81;
		time_c++;
		ave_L = (float)Lempty_c/(float)time_c;
		ave_H = (float)Hempty_c/(float)time_c;
		ave_K = (float)Kempty_c/(float)time_c;

		char data[1024] = {};
		char data1[1024] = {};//add data
		FILE *fd_dema;
		fd_dema = fopen("/www/empty_con.txt", "w+b");

		sprintf(data,"%f,%f,%f",ave_L,ave_H,ave_K);
		sprintf(data1,"%s",data);
		fputs(data1,fd_dema);
		fclose(fd_dema);
		FILE *fd_st;
		fd_st = fopen("/www/state", "w");
					fputs("d",fd_st);
		system("killall -9 guo");
	}
}
void oilcon()
{
	static unsigned long int Loil_c;
	static unsigned long int Hoil_c;
	static unsigned long int Koil_c;
	static unsigned long int time_c1;
	if(time_c1 < ptr->mode_time*60000/ptr->st){
		Loil_c = log(ptr->emptylowCount/(ptr->spidata[0]*256+ptr->spidata[1]))/ptr->absorptionDistance + Loil_c;
		Hoil_c = log(ptr->emptyhighCount/(ptr->spidata[2]*256+ptr->spidata[3]))/ptr->absorptionDistance + Hoil_c;
		Koil_c = log(ptr->emptykCount/(ptr->spidata[4]*256+ptr->spidata[5]))/ptr->absorptionDistance + Koil_c;
		time_c1++;
	}
	else{
		Loil_c = Loil_c - Koil_c*ptr->k31 - ptr->escaping*Hoil_c;
		Hoil_c = Hoil_c - Koil_c*ptr->k81;
		time_c1++;
		Loil_c = Loil_c/time_c1;
		Hoil_c = Hoil_c/time_c1;

		char data[1024] = {};
		char data1[1024] = {};//add data
		FILE *fd_dema1;
		fd_dema1 = fopen("/www/oil_con.txt", "w+b");

		sprintf(data,"%ld,%ld",Loil_c,Hoil_c);
		sprintf(data1,"%s",data);
		fputs(data1,fd_dema1);
		fclose(fd_dema1);

		system("killall -9 guo");
	}
}
void gascon()
{
	static unsigned long int Lgas_c;
	static unsigned long int Hgas_c;
	static unsigned long int Kgas_c;
	static unsigned long int time_c2;
	if(time_c2 < ptr->mode_time*60000/ptr->st){
		Lgas_c = log(ptr->emptylowCount/(ptr->spidata[0]*256+ptr->spidata[1]))/ptr->absorptionDistance + Lgas_c;
		Hgas_c = log(ptr->emptyhighCount/(ptr->spidata[2]*256+ptr->spidata[3]))/ptr->absorptionDistance + Hgas_c;
		Kgas_c = log(ptr->emptykCount/(ptr->spidata[4]*256+ptr->spidata[5]))/ptr->absorptionDistance + Kgas_c;
		time_c2++;
	}
	else{
		Lgas_c = Lgas_c - Kgas_c*ptr->k31 - ptr->escaping*Hgas_c;
		Hgas_c = Hgas_c - Kgas_c*ptr->k81;
		time_c2++;
		Lgas_c = Lgas_c/time_c2;
		Hgas_c = Hgas_c/time_c2;

		char data[1024] = {};
		char data1[1024] = {};//add data
		FILE *fd_dema2;
		fd_dema2 = fopen("/www/gas_con.txt", "w+b");

		sprintf(data,"%ld,%ld",Lgas_c,Hgas_c);
		sprintf(data1,"%s",data);
		fputs(data1,fd_dema2);
		fclose(fd_dema2);

		system("killall -9 guo");
	}
}
void watercon()
{
	static unsigned long int Lwater_c;
	static unsigned long int Hwater_c;
	static unsigned long int Kwater_c;
	static unsigned long int time_c3;
	if(time_c3 < ptr->mode_time*60000/ptr->st){
		Lwater_c = log(ptr->emptylowCount/(ptr->spidata[0]*256+ptr->spidata[1]))/ptr->absorptionDistance + Lwater_c;
		Hwater_c = log(ptr->emptyhighCount/(ptr->spidata[2]*256+ptr->spidata[3]))/ptr->absorptionDistance + Hwater_c;
		Kwater_c = log(ptr->emptykCount/(ptr->spidata[4]*256+ptr->spidata[5]))/ptr->absorptionDistance + Kwater_c;
		time_c3++;
	}
	else{
		Lwater_c = Lwater_c - Kwater_c*ptr->k31 - ptr->escaping*Hwater_c;
		Hwater_c = Hwater_c - Kwater_c*ptr->k81;
		time_c3++;
		Lwater_c = Lwater_c/time_c3;
		Hwater_c = Hwater_c/time_c3;

		char data[1024] = {};
		char data1[1024] = {};//add data
		FILE *fd_dema3;
		fd_dema3 = fopen("/www/water_con.txt", "w+b");

		sprintf(data,"%ld,%ld",Lwater_c,Hwater_c);
		sprintf(data1,"%s",data);
		fputs(data1,fd_dema3);
		fclose(fd_dema3);

		system("killall -9 guo");
	}
}



