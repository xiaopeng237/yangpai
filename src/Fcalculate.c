/*
 * Fcalculate.c
 *
 *  Created on: 2016-6-13
 *      Author: vmuser
 */
#include "Fcalculate.h"
#include "readpara.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <linux/watchdog.h>
#include "data.h"
#include "shmdata.h"
#define K   273.15
ResultPVT 				s_ResultPVT;
extern Cpar				cpar;
extern flowR 			flowresult;
extern int 			fd_dog;
extern flag				flags;
extern PVT_BlackOil		PVT_BlackOils;
extern PVT_Normal		PVT_Normals;
extern PFC_Three		PFC_Threes;
extern Flow_result		Flow_results;
extern	corr			corrs;
extern	cpm				cpms;
extern datetime 		m_time;
extern int 			win_C[200][6];//计算用的二维数组
extern struct shared_use_st *shared;//指向shm
float Bg;
int tests()
{
	int i;

	for(i=0;i<200;i++)
	{
		cpar.Measure_DP[i]=shared->ad1115[i][2];
		cpar.Win1[i]=win_C[i][1];
		cpar.Win2[i]=win_C[i][2];
		cpar.Win5[i]=win_C[i][4];
		if(cpar.Win1[i]==0)
			cpar.Win1[i]=1;
		if(cpar.Win2[i]==0)
			cpar.Win2[i]=1;
		if(cpar.Win5[i]==0)
			cpar.Win5[i]=1;
	}
	cpar.Measure_Pre=shared->ad1115[199][1];
	cpar.Measure_Tem=shared->ad1115[199][0];
	return 0;
}
/*
 * 数据计算入口；
 * 每秒有200组数据，分别计算然后累加
 * 没分钟输出一个logfile数据并存储
 */
int fcalculate()
{
	int i;
	//text： 每分钟流量

	/*
	 * ad1115数据存入计算单元
	 */
	static int math_C=0;
// pvt计算

	tests();
	PVTcalculations();//计算PVT
	PhaseFractionCalculates();//计算相分率PFC
	//text();
	//MFMC();


	if(math_C<60)//（）每秒数据(五个窗口计数；温度；压力；差压)计算200次，数据结果累加输出
	{
		flowresult.fgas		=	Flow_results.Qm_Gas		+	flowresult.fgas;
		flowresult.foil		=	Flow_results.Qm_Oil		+	flowresult.foil;
		flowresult.fwater	=	Flow_results.Qm_Water	+	flowresult.fwater;
		flowresult.OMF		=	Flow_results.OMF		+	flowresult.OMF;
		flowresult.GMF		=	Flow_results.GMF		+	flowresult.GMF;
		flowresult.WMF		=	Flow_results.WMF		+	flowresult.WMF;

		math_C++;
		if(math_C==60)//每分钟数据；数据存入mysql；生成流量表格.json；生成曲线文件；
		{
			/*
			 * 每分钟计算一次
			 */
			//CPM1();
			CPM2();

			math_C=0;
			flow_data_w();
			flow1_data_w();
			flow2_data_w();
			flow3_data_w();
			flow4_data_w();
			flow5_data_w();
			flow_table_w();
			printf("=======%d\n",flags.flag_log);
			if(flags.flag_log==2)
			{
				//shared->W_log=1;
				W_logfile();
			}
			flowresult.fgas=0;
			flowresult.foil=0;
			flowresult.fwater=0;
			flowresult.OMF=0;
			flowresult.GMF=0;
			flowresult.WMF=0;
			W_FlowData();
//printf("json\n");
		}

	}
	else{
		printf("math_C error");
		math_C=0;
	}

///看门狗喂狗

	if(flags.flag_dog!=0){
		int dummy;
		ioctl(fd_dog, WDIOC_KEEPALIVE, &dummy);
	}
	//CPS();
	return 0;
}


void PVTcalculations(void)
{
//计算PVT
	int PVTmodel = 1;
				//(int) m_modelParameter.nPVTModelSelect;

	switch(PVTmodel)
	{
	    case 0 	:
	    	normalPVT();
					break;
		case 1  :
			//blackOilModels();
					break;
		case 2  :
			//customModel();
		            break;
		case 3  :
			//PVTsimModel();
					break;
		case 4  :
			//PVTsimModel();
					break;
		default :   break;

	}
}
void normalPVT(void)
{
    float  betaO = PVT_Normals.beatO;//液的膨胀系数？？？单位pa
    float  betaW = PVT_Normals.beatW;//液的膨胀系数？？？单位pa
	float  P0    = PVT_Normals.P0;//标定气压adc赋值
	float  P     = cpar.Measure_Pre;//工况气压
	float  t0    = PVT_Normals.T0;//标定温度
	float  t     = cpar.Measure_Tem;//工况温度

	float  t_K   = t + K;//摄氏度变华氏度
	float  t0_K  = t0+ K;
//    float  temperatureK = s_nMVT.temperature +K;
	float  oilDensitySC = PVT_Normals.oilDensitySC;//标况密度哪里得到？？？？？？
	float  waterDensitySC = PVT_Normals.waterDensitySC;
	float  gasDensitySC = PVT_Normals.gasDensitySC;

	float  oilDensityLC,waterDensityLC,gasDensityLC;//工况密度

    float	Bo	= 	1;//1/(1-betaO *(t-t0));//油气水三者的工况标况密度比
    float	Bw 	=	1;//1/(1-betaW *(t-t0));
    		Bg	= 	(P0*t_K)/(P*t0_K);
    float	Z 	= 	1;//气体压缩因子
    float	Rs 	=	0;//溶解气  为什么是0？

	oilDensityLC =  oilDensitySC/Bo ;//转化为工况密度
    waterDensityLC = waterDensitySC/Bw;
	gasDensityLC  = gasDensitySC/Bg;

	s_ResultPVT.oilDensityLC = oilDensityLC;
	s_ResultPVT.waterDensityLC = waterDensityLC;
	s_ResultPVT.gasDensityLC = gasDensityLC;
//	printf("%f,%f,%f",s_measureResult.oilDensityLC,s_measureResult.waterDensityLC,s_measureResult.gasDensityLC);
}
void blackOilModels(void)
{
	float deltaG 			=	PVT_BlackOils.Gr;
	float Gr 				= 	PVT_BlackOils.Gr;
	float Mc 				= 	PVT_BlackOils.nMC;//m_modelParameter.nMC;
	float Mn 				= 	PVT_BlackOils.nMn;//m_modelParameter.nMn;
	float Mc1 				= 	PVT_BlackOils.nMC1;//m_modelParameter.nMC1;
	float Zn  				=	PVT_BlackOils.nZn;//m_modelParameter.nZn;
	float P0 				= 	PVT_BlackOils.localPressure/1000;//m_publicParameter.localBarometricPressure/1000;
	float betaO 			= 	PVT_BlackOils.oilExpansionFactor;//m_publicParameter.oilExpansionFactor;
	float oilDensitySC 		= 	PVT_BlackOils.oilDensitySC;//m_publicParameter.oilDensitySC;
	float waterDensitySC 	= 	PVT_BlackOils.waterDensitySC;//m_publicParameter.waterDensitySC;
	float gasDensitySC 		= 	PVT_BlackOils.gasDensitySC;//m_publicParameter.gasDensitySC;

	float P   				= 	cpar.Measure_Pre/1000;//s_nMVT.pressure/1000;
	float t   				= 	cpar.Measure_Tem;//s_nMVT.temperature;
	float Fp,Ft,Kp,Kt,tao,E,m,n,B,b,D,Fz,Z;
	float  oilDensityLC,waterDensityLC,gasDensityLC;
	if(Gr>0.75 ||Mc>0.15 || Mn>0.15)
	{
		Fp=671.4/(891.11-172.56*Gr+443.04*Mc-232.32*Mn-122.52*Mc1);
		Ft=359.46/(327.77+214.82*Gr-144.12*Mc-319.52*Mn-102.78*Mc1);
	}
	else
	{
		Kp=(Mc-0.392*Mn)*100;
		Kt=(Mc+1.681*Mn)*100;
		Fp=156.47/(160.8-7.22*deltaG+Kp);
		Ft=226.29/(99.15+211.9*deltaG-Kt);
	}
	//	float Pj=145.04*P*Fp;
	float Pj=145.04*(P-0.101325)*Fp;//0.101325是大气压
	float Tj=(1.8*t+492)*Ft-460;
	float H=(Pj+14.7)/1000;
	tao=(Tj+460)/500;
	if( tao >= 1.09 && tao< 1.4 && P <=13.79 )
	{
		float e0 =tao-1.09;
		float e1 =-20 * e0;
		float e2 =2.17 + 1.4 * pow(e0,0.5)-H;
		E = 1 - 0.00075*pow(H,2.3)*exp(e1)-0.0011*pow(e0,0.5)*pow(H,2)*pow(e2,2);
	}
	else if ( tao<1.09&&tao>=0.88&&P<=13.79&&P>8.963)
	{
		float e0 =1.09 -tao;
		float e1 =-20 * e0;
		E = 1- 0.00075 * pow(H,2.3)*(2- exp(e1))+0.455 *(200* pow(e0,6)-0.03249*e0+2.0167*pow(e0,2)-18.028*pow(e0,3)+42.844*pow(e0,4))*(H-1.3)*(4.01952-pow(H,2));
	}
	else if (tao<1.09&&tao>=0.84&&P<=8.963)
	{
		float e0 =1.09 -tao;
		float e1 =-20 * e0;
		E = 1-0.00075*pow(H,2.3)*(2-exp(e1))-1.317*pow(e0,4)*H*(1.69-pow(H,2));
		//printf("E=%f H=%f e1=%f e0=%f\n",E,H,e1,e0);
	}
	else
		E = 0;
	m=0.0330378*pow(tao,-2)-0.0221323*pow(tao,-3)+0.0161353*pow(tao,-5);
	n=(-0.133185*pow(tao,-1)+0.265827*pow(tao,-2)+0.0457697*pow(tao,-4))/m;
	B = (3-m*pow(n,2))/(9*m*pow(H,2));
	b = (9*n-2*m*pow(n,3))/(54*m*pow(H,3))-E/(2*m*pow(H,2));
	//printf("n=%f m=%f H=%f E=%f\n",n,m,H,E);
	float D0 = pow(b,2)+pow(B,3);
	//printf("b=%f B=%f\n",b,B);
	float D1 = sqrt(D0)+b;
	//printf("D0=%f b=%f\n",D0,b);
	float D2 =1.0/3;
	D = pow(D1,D2);
	//printf("D1=%f D2=%f\n",D1,D2);
	float Fz0 = B/D-D+n/(3*H);
	//printf("B=%f D=%f n=%f H=%f\n",B,D,n,H);

	Fz = sqrt(Fz0)/(1+0.00132/pow(tao,3.25));
	//printf("Fz0=%f tao=%f\n",Fz0,tao);

	Z =Zn/pow(Fz,2);
	//printf("Zn=%f Fz=%f\n",Zn,Fz);

	float T0 = PVT_BlackOils.referenceTempertaureSC +K;
	float Pb = PVT_BlackOils.nPb/1000;
	float T  =t +K;
	float Bg = Z*P0*T/(P*T0);
	//printf("z=%f P0=%f T=%f P=%f T0=%f\n",Z,P0,T,P,T0);

	float deltaO = PVT_BlackOils.oilDensitySC/1000;
	float Rs,Mo,Bo,Bw;
	if(P<Pb )
	{
		if(deltaO <= 0.966)
		{
			float Rs0 = 1.77 *pow(deltaO,-1)-0.001638*t-1.67;
			float Rs1 =P * pow(10,Rs0);
			Rs = 2.2*deltaG*pow(Rs1,1.205);
		}
		else
		{
			float oilDen1556 = deltaO /(1-betaO*(t-15.56));
			Mo = 1.806*oilDen1556-1246;
			float Yg0 = 118.69*P*deltaG/T+0.891;
			float Yg  = 0.826*log10(Yg0);
			Rs = 2.3674 * pow(10,4)*Yg*deltaO/(1-Yg)/Mo;
		}
	}
	else
	{
		float oilDen1556 = deltaO /(1-betaO*(t-15.56));
		Mo = 1.806*oilDen1556-1246;
		float Yg0 = 118.69*Pb*deltaG/T+0.891;
		float Yg  = 0.826*log10(Yg0);
		Rs = 2.3674 * pow(10,4)*Yg*deltaO/(1-Yg)/Mo;
	}

	if(P<Pb)
	{
		float F0 = deltaG/deltaO;
		float F  = 5.62 * Rs * pow(F0,0.5)+2.25*t+40;
		Bo = 0.972 +0.000147 * pow(F,1.175);
	}
	else if (P==Pb)
	{
		float F0 = deltaG/deltaO;
		float F  = 5.62 * Rs * pow(F0,0.5)+2.25*t+40;
		Bo = 0.972 +0.000147 * pow(F,1.175);
	}
	else
	{
		float API = 141.5/deltaO -131.5;
		float deltaG1147 = deltaG*(1-0.0152*API);
		float Co = (-1433+28.075*Rs+17.2*T-1180*deltaG1147+12.61*API)/(14540000*P);
		float Bo0 =145.4*Co*(Pb-P);
		float F0 = deltaG/deltaO;
		float F  = 5.62 * Rs * pow(F0,0.5)+2.25*t+40;
		float Bob = 0.972 +0.000147 * pow(F,1.175);
		Bo = Bob * exp(Bo0);
	}
	float deltaGs = Rs * (0.00379*deltaO-0.00379)-4.08779*deltaO+4.43818;
	Bw = 1.0088 -4.4748*P*pow(10,-4)+6.2666*P*pow(10,-7);

	cpar.Bo		= 	Bo;
	cpar.Bw 	=	Bw;
	cpar.Bg		= 	Bg;
	cpar.Z 		= 	Z;
	cpar.Rs		=	Rs;
//	s_resultPVT.Rs 	=	Rs;

	oilDensityLC = (oilDensitySC+ Rs *deltaGs*gasDensitySC/Gr)/Bo;
	gasDensityLC  = gasDensitySC/Bg;
	waterDensityLC = waterDensitySC/Bw;
	//printf("result is %f %f %f %f %f %f %f %f\n",Bo,Bw,Bg,Z,Rs,oilDensityLC,waterDensityLC,gasDensityLC);
	cpar.oilDensityLC 		= 	oilDensityLC;
	cpar.waterDensityLC 	= 	waterDensityLC;
	cpar.gasDensityLC 		= 	gasDensityLC;

	Flow_results.oilDensityLC		=	oilDensityLC;
	Flow_results.waterDensityLC		=	waterDensityLC;
	Flow_results.gasDensityLC		=	gasDensityLC;
	//printf("Flow_results.oilDensityLC=%f Flow_results.waterDensityLC=%f Flow_results.gasDensityLC=%f\n",Flow_results.oilDensityLC,Flow_results.waterDensityLC	,Flow_results.gasDensityLC);

}
void PhaseFractionCalculates(void)
{
	//计算相分率PFC
	int PFCmodel = 0;//PFC模式选择

	//应该有类似k31 k81的参数修正N计数

	switch(PFCmodel)
	{
		case 0 	:   //DoubleEnergyPFC();
					break;
		case 1  :	ThirdEnergyPFC();
					break;
		case 2  :	LowCountThirdEnergyPFC();
		            break;
		case 3  :	LowCountDoubleEnergyPFC();
					break;
		default :   break;

	 }
}
void ThirdEnergyPFC(void)
{
	int i;
	float a_w1			=	PFC_Threes.Energy1_Water;//energy1.waterAbsorptionCoefficient;//油气水三个峰的各自吸收系数由实验获得
	float a_o1			=	PFC_Threes.Energy1_Oil;//energy1.oilAbsorptionCoefficient;
	float a_g1			=	PFC_Threes.Energy1_Gas;//energy1.gasAbsorptionCoefficient;
	float a_e1			=	PFC_Threes.Energy1_Empty;//energy1.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w1,a_o1,a_g1,a_e1);

	float a_w2			=	PFC_Threes.Energy2_Water;//energy2.waterAbsorptionCoefficient;
	float a_o2			=	PFC_Threes.Energy2_Oil;//energy2.oilAbsorptionCoefficient;
	float a_g2			=	PFC_Threes.Energy2_Gas;//energy2.gasAbsorptionCoefficient;
	float a_e2			=	PFC_Threes.Energy2_Empty;//energy2.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w2,a_o2,a_g2,a_e2);
	float a_w5			=	PFC_Threes.Energy5_Water;//energy5.waterAbsorptionCoefficient;
	float a_o5			=	PFC_Threes.Energy5_Oil;//energy5.oilAbsorptionCoefficient;
	float a_g5			=	PFC_Threes.Energy5_Gas;//energy5.gasAbsorptionCoefficient;
	float a_e5			=	PFC_Threes.Energy5_Empty;//energy5.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w5,a_o5,a_g5,a_e5);

	float f1			=	PFC_Threes.f1;//m_publicParameter.f1;//第二股射线和第三股射线相对于第一股射线的强度比值
	float f2			=	PFC_Threes.f2;//m_publicParameter.f2;

	float C				=	PFC_Threes.C;//1;//节流型流量计流出系数
	float epsilon		=	PFC_Threes.epsilon;//1;//流体压缩修正因子ε,对于液体为1
	float D				=	PFC_Threes.Diameter;//1;//管道直径，射线测量厚度
	float beta			=	PFC_Threes.beta;//1;//节流型流量计管道直径比

	float deltaP;//1;//差压测量值ΔP

	float k1;
	float k2;
	float k3;
	float k4;

	float d1;//log(Nx2/f1*Nx1) Nx1 2 3 是三种能量投射强度
	float d2;//log(Nx3/f2*Nx1)

	float theta;//θ

	float Q_o;
	float Q_g;
	float Q_w;

	float OMF;
	float GMF;
	float WMF;

	float S	;
	float rho_mix;
	float Qm;

	float Qm_o=0.0;
	float Qm_g=0.0;
	float Qm_w=0.0;

	for(i=0;i<200;i++)
	{
		deltaP		=	cpar.Measure_DP[i];//1;//差压测量值ΔP
		if(cpar.Win1[i]==0){
			cpar.Win1[i]=1;
		}
		if(cpar.Win2[i]==0){
			cpar.Win2[i]=1;
		}
		if(cpar.Win5[i]==0){
			cpar.Win5[i]=1;
		}

		k1			=	(a_g1-a_g2)/(a_o1-a_o2);
		k2			=	(a_w1-a_w2)/(a_o1-a_o2);
		k3			=	(a_g1-a_g5)/(a_o1-a_o5);
		k4			=	(a_w1-a_w5)/(a_o1-a_o5);
	//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",k1,k2,k3,k4);

		d1			=	1/(a_o1-a_o2)*log(cpar.Win2[i]/(f1*cpar.Win1[i]));//log(Nx2/f1*Nx1) Nx1 2 3 是三种能量投射强度
		d2			=	1/(a_o1-a_o5)*log(cpar.Win5[i]/(f2*cpar.Win1[i]));//log(Nx3/f2*Nx1)
	//	printf("d1=%f,d2=%f\n",d1,d2);
		theta  		=	(M_PI/4)*(pow(((C/(pow((1-(pow(beta,4))),0.5)))*epsilon*D),2))*deltaP;//θ
	//	printf("theta =%f\n",theta);
		Q_o			=	theta-((((k2-1)*(d1-d2))+((k4-k2)*(d1-theta)))/(((k2-1)*(k1-k3))+((k4-k2)*(k1-1))))-((((k3-1)*(d1-d2))+((k3-k1)*(d1-theta)))/(((k3-1)*(k2-k4))+((k3-k1)*(k4-1))));
		Q_g			=	((((k2-1)*(d1-d2))+((k4-k2)*(d1-theta)))/(((k2-1)*(k1-k3))+((k4-k2)*(k1-1))));
		Q_w			=	((((k3-1)*(d1-d2))+((k3-k1)*(d1-theta)))/(((k3-1)*(k2-k4))+((k3-k1)*(k4-1))));
	//	printf("Q-o=%f,Q-g=%f,Q-w=%f\n",Q_o,Q_g,Q_w);
		OMF			=	Q_o/(Q_o+Q_w+Q_g);
		GMF			=	Q_g/(Q_o+Q_w+Q_g);
		WMF			=	Q_w/(Q_o+Q_w+Q_g);

		S			=	(M_PI/4)*pow(D,2);
		rho_mix		=	(Q_o+Q_g+Q_w)/S;
		Qm			=	(C/(pow((1-pow(beta,4)),0.5)))*epsilon*(M_PI/4)*pow(D,2)*pow(deltaP*rho_mix,0.5);

		Qm_o			=	Qm*OMF+Qm_o;
		Qm_g			=	Qm*GMF+Qm_g;
		Qm_w			=	Qm*WMF+Qm_w;
		Flow_results.Qm_Oils[i]		=	Qm*OMF;
		Flow_results.Qm_Gass[i]		=	Qm*GMF;
		Flow_results.Qm_Waters[i]	=	Qm*WMF;
	}
	Flow_results.Qm_Oil			=	Qm_o;
	Flow_results.Qm_Gas			=	Qm_g;
	Flow_results.Qm_Water		=	Qm_w;
	Flow_results.OMF			=   Qm_o/(Qm_o+Qm_g+Qm_w);
	Flow_results.GMF			=   Qm_g/(Qm_o+Qm_g+Qm_w);
	Flow_results.WMF			=   Qm_w/(Qm_o+Qm_g+Qm_w);
	//printf("Q-o=%f,Q-g=%f,Q-w=%f\n",Qm_o,Qm_g,Qm_w);
}
void LowCountThirdEnergyPFC(void)
{
	int i;
	float a_w1			=	PFC_Threes.Energy1_Water;//energy1.waterAbsorptionCoefficient;//油气水三个峰的各自吸收系数由实验获得
	float a_o1			=	PFC_Threes.Energy1_Oil;//energy1.oilAbsorptionCoefficient;
	float a_g1			=	PFC_Threes.Energy1_Gas;//energy1.gasAbsorptionCoefficient;
	float a_e1			=	PFC_Threes.Energy1_Empty;//energy1.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w1,a_o1,a_g1,a_e1);

	float a_w2			=	PFC_Threes.Energy2_Water;//energy2.waterAbsorptionCoefficient;
	float a_o2			=	PFC_Threes.Energy2_Oil;//energy2.oilAbsorptionCoefficient;
	float a_g2			=	PFC_Threes.Energy2_Gas;//energy2.gasAbsorptionCoefficient;
	float a_e2			=	PFC_Threes.Energy2_Empty;//energy2.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w2,a_o2,a_g2,a_e2);
	float a_w5			=	PFC_Threes.Energy5_Water;//energy5.waterAbsorptionCoefficient;
	float a_o5			=	PFC_Threes.Energy5_Oil;//energy5.oilAbsorptionCoefficient;
	float a_g5			=	PFC_Threes.Energy5_Gas;//energy5.gasAbsorptionCoefficient;
	float a_e5			=	PFC_Threes.Energy5_Empty;//energy5.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w5,a_o5,a_g5,a_e5);

	float f1			=	PFC_Threes.f1;//m_publicParameter.f1;//第二股射线和第三股射线相对于第一股射线的强度比值
	float f2			=	PFC_Threes.f2;//m_publicParameter.f2;

	float C				=	PFC_Threes.C;//1;//节流型流量计流出系数
	float epsilon		=	PFC_Threes.epsilon;//1;//流体压缩修正因子ε,对于液体为1
	float D				=	PFC_Threes.Diameter;//1;//管道直径，射线测量厚度
	float beta			=	PFC_Threes.beta;//1;//节流型流量计管道直径比

	float deltaP;//1;//差压测量值ΔP

	float k1;
	float k2;
	float k3;
	float k4;

	float d1;//log(Nx2/f1*Nx1) Nx1 2 3 是三种能量投射强度
	float d2;//log(Nx3/f2*Nx1)

	float theta;//θ

	float Q_o;
	float Q_g;
	float Q_w;

	float OMF;
	float GMF;
	float WMF;

	float S	;
	float rho_mix;
	float Qm;

	float Qm_o=0.0;
	float Qm_g=0.0;
	float Qm_w=0.0;

	int n;//1s数据循环次数
	int j;
	int s=50;//每个计算单元数据累加时间5ms的倍数；日后从数据库赋值
	n=1000/s;
	for(i=0;i<200;i++)
	{
		cpar.Win1[i]=0;
		cpar.Win2[i]=0;
		cpar.Win5[i]=0;
		cpar.Measure_DP[i]=0;
	}
	for(i=0;i<n;i++)
	{
		for(j=0;j<(s/5);j++)
		{
			cpar.Win1[i]		=	cpar.Win1[i]		+	win_C[n*(s/5)+j][1];
			cpar.Win2[i]		=	cpar.Win2[i]		+	win_C[n*(s/5)+j][2];
			cpar.Win5[i]		=	cpar.Win5[i]		+	win_C[n*(s/5)+j][4];
			cpar.Measure_DP[i]	=	cpar.Measure_DP[i]	+	shared->ad1115[n*(s/5)+j][2];
		}
		cpar.Measure_DP[i]=cpar.Measure_DP[i]/(s/5);
	}


	for(i=0;i<n;i++)
	{
		deltaP		=	cpar.Measure_DP[i];//1;//差压测量值ΔP
		if(cpar.Win1[i]==0){
			cpar.Win1[i]=1;
		}
		if(cpar.Win2[i]==0){
			cpar.Win2[i]=1;
		}
		if(cpar.Win5[i]==0){
			cpar.Win5[i]=1;
		}

		k1			=	(a_g1-a_g2)/(a_o1-a_o2);
		k2			=	(a_w1-a_w2)/(a_o1-a_o2);
		k3			=	(a_g1-a_g5)/(a_o1-a_o5);
		k4			=	(a_w1-a_w5)/(a_o1-a_o5);
	//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",k1,k2,k3,k4);

		d1			=	1/(a_o1-a_o2)*log(cpar.Win2[i]/(f1*cpar.Win1[i]));//log(Nx2/f1*Nx1) Nx1 2 3 是三种能量投射强度
		d2			=	1/(a_o1-a_o5)*log(cpar.Win5[i]/(f2*cpar.Win1[i]));//log(Nx3/f2*Nx1)
	//	printf("d1=%f,d2=%f\n",d1,d2);
		theta  		=	(M_PI/4)*(pow(((C/(pow((1-(pow(beta,4))),0.5)))*epsilon*D),2))*deltaP;//θ
	//	printf("theta =%f\n",theta);
		Q_o			=	theta-((((k2-1)*(d1-d2))+((k4-k2)*(d1-theta)))/(((k2-1)*(k1-k3))+((k4-k2)*(k1-1))))-((((k3-1)*(d1-d2))+((k3-k1)*(d1-theta)))/(((k3-1)*(k2-k4))+((k3-k1)*(k4-1))));
		Q_g			=	((((k2-1)*(d1-d2))+((k4-k2)*(d1-theta)))/(((k2-1)*(k1-k3))+((k4-k2)*(k1-1))));
		Q_w			=	((((k3-1)*(d1-d2))+((k3-k1)*(d1-theta)))/(((k3-1)*(k2-k4))+((k3-k1)*(k4-1))));
	//	printf("Q-o=%f,Q-g=%f,Q-w=%f\n",Q_o,Q_g,Q_w);
		OMF			=	Q_o/(Q_o+Q_w+Q_g);
		GMF			=	Q_g/(Q_o+Q_w+Q_g);
		WMF			=	Q_w/(Q_o+Q_w+Q_g);

		S			=	(M_PI/4)*pow(D,2);
		rho_mix		=	(Q_o+Q_g+Q_w)/S;
		Qm			=	(C/(pow((1-pow(beta,4)),0.5)))*epsilon*(M_PI/4)*pow(D,2)*pow(deltaP*rho_mix,0.5);

		Qm_o			=	Qm*OMF+Qm_o;
		Qm_g			=	Qm*GMF+Qm_g;
		Qm_w			=	Qm*WMF+Qm_w;
		Flow_results.Qm_Oils[i]		=	Qm*OMF;
		Flow_results.Qm_Gass[i]		=	Qm*GMF;
		Flow_results.Qm_Waters[i]	=	Qm*WMF;
	}
	Flow_results.Qm_Oil			=	Qm_o;
	Flow_results.Qm_Gas			=	Qm_g;
	Flow_results.Qm_Water		=	Qm_w;
	Flow_results.OMF			=   Qm_o/(Qm_o+Qm_g+Qm_w);
	Flow_results.GMF			=   Qm_g/(Qm_o+Qm_g+Qm_w);
	Flow_results.WMF			=   Qm_w/(Qm_o+Qm_g+Qm_w);
	//printf("Q-o=%f,Q-g=%f,Q-w=%f\n",Qm_o,Qm_g,Qm_w);

}
void LowCountDoubleEnergyPFC(void)
{
	int i;
	float a_w1			=	PFC_Threes.Energy1_Water;//energy1.waterAbsorptionCoefficient;//油气水三个峰的各自吸收系数由实验获得
	float a_o1			=	PFC_Threes.Energy1_Oil;//energy1.oilAbsorptionCoefficient;
	float a_g1			=	PFC_Threes.Energy1_Gas*Bg;//energy1.gasAbsorptionCoefficient;
	float a_e1			=	PFC_Threes.Energy1_Empty;//energy1.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w1,a_o1,a_g1,a_e1);

	float a_w2			=	PFC_Threes.Energy2_Water;//energy2.waterAbsorptionCoefficient;
	float a_o2			=	PFC_Threes.Energy2_Oil;//energy2.oilAbsorptionCoefficient;
	float a_g2			=	PFC_Threes.Energy2_Gas;//energy2.gasAbsorptionCoefficient;
	float a_e2			=	PFC_Threes.Energy2_Empty;//energy2.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w2,a_o2,a_g2,a_e2);
	float a_w5			=	PFC_Threes.Energy5_Water;//energy5.waterAbsorptionCoefficient;
	float a_o5			=	PFC_Threes.Energy5_Oil;//energy5.oilAbsorptionCoefficient;
	float a_g5			=	PFC_Threes.Energy5_Gas*Bg;//energy5.gasAbsorptionCoefficient;
	float a_e5			=	PFC_Threes.Energy5_Empty;//energy5.emptyPipeCount;
//	printf("k1=%f,k2=%f,k3=%f,k4=%f\n",a_w5,a_o5,a_g5,a_e5);

	float f1			=	PFC_Threes.f1;//m_publicParameter.f1;//第二股射线和第三股射线相对于第一股射线的强度比值
	float f2			=	PFC_Threes.f2;//m_publicParameter.f2;

	float C				=	PFC_Threes.C;//1;//节流型流量计流出系数
	float epsilon		=	PFC_Threes.epsilon;//1;//流体压缩修正因子ε,对于液体为1
	float D				=	PFC_Threes.Diameter;//1;//管道直径，射线测量厚度
	float beta			=	PFC_Threes.beta;//1;//节流型流量计管道直径比

	float deltaP;//1;//差压测量值ΔP


	float OVF;
	float GVF;
	float Qv;
	float Pmix=1.0;//PVT


	float Qm_o=0.0;
	float Qm_g=0.0;
	float Qm_w=0.0;

	int n;//1s数据循环次数
	int j;
	int s=PVT_Normals.ntime;//每个计算单元数据累加时间5ms的倍数；日后从数据库赋值
	n=1000/s;
	for(i=0;i<200;i++)
	{
		cpar.Win1[i]=0;
		cpar.Win2[i]=0;
		cpar.Win5[i]=0;
		cpar.Measure_DP[i]=0;
	}
	for(i=0;i<n;i++)
	{
		for(j=0;j<(s/5);j++)
		{
			cpar.Win1[i]		=	cpar.Win1[i]		+	win_C[n*(s/5)+j][1];
			cpar.Win2[i]		=	cpar.Win2[i]		+	win_C[n*(s/5)+j][2];
			cpar.Win5[i]		=	cpar.Win5[i]		+	win_C[n*(s/5)+j][4];
			cpar.Measure_DP[i]	=	cpar.Measure_DP[i]	+	shared->ad1115[n*(s/5)+j][2];
		}
		cpar.Measure_DP[i]=cpar.Measure_DP[i]/(s/5);
	}


	for(i=0;i<n;i++)
	{
		deltaP		=	cpar.Measure_DP[i];//1;//差压测量值ΔP
		if(cpar.Win1[i]==0){
			cpar.Win1[i]=1;
		}
		if(cpar.Win2[i]==0){
			cpar.Win2[i]=1;
		}
		if(cpar.Win5[i]==0){
			cpar.Win5[i]=1;
		}

		OVF			=	((log(cpar.Win5[i]/(f2*cpar.Win1[i]))-D*(a_g1-a_g5))/(a_o1-a_o5-a_g1+a_g5))/D;
		GVF			=	1-OVF;

		Pmix		=	OVF*s_ResultPVT.oilDensityLC+GVF*s_ResultPVT.gasDensityLC;
		Qv			=	(M_PI/4)*(C/(pow((1-(pow(beta,4))),0.5)))*D*D*epsilon*pow((2*deltaP)/Pmix,0.5);


		Qm_o		=	Qv*OVF+Qm_o;
		Qm_g		=	Qv*GVF+Qm_g;
		Qm_w		=	0;

		Flow_results.Qm_Oils[i]		=	Qv*OVF;
		Flow_results.Qm_Gass[i]		=	Qv*GVF;
		Flow_results.Qm_Waters[i]	=	0;
	}
	Flow_results.Qm_Oil			=	Qm_o;
	Flow_results.Qm_Gas			=	Qm_g;
	Flow_results.Qm_Water		=	Qm_w;
	Flow_results.OMF			=   Qm_o/(Qm_o+Qm_g+Qm_w);
	Flow_results.GMF			=   Qm_g/(Qm_o+Qm_g+Qm_w);
	Flow_results.WMF			=   Qm_w/(Qm_o+Qm_g+Qm_w);
	//printf("Q-o=%f,Q-g=%f,Q-w=%f\n",Qm_o,Qm_g,Qm_w);

}
void CPM(void){
	//计算原始量6个
	int 	k31_CPM;//31能量计数每分钟 硬件获取
	int 	k81_CPM;//81能量计数每分钟 硬件获取
	int 	k356_CPM;//356能量计数每分钟 硬件获取
	float 	DP_CPM;//差压数值 硬件获取
	float 	P_CPM;//绝压数值 硬件获取
	float 	T_P_CPM;//绝压温度 硬件获取
	float 	T_DP_CPM;//差压温度 硬件获取

	float 	P_CPM_A;//绝压数值修正系数 数据库读取
	float 	P_CPM_B;//绝压数值修正系数 数据库读取
	float 	DP_CPM_A;//差压数值修正系数 数据库读取
	float 	DP_CPM_B;//差压数值修正系数 数据库读取
	float 	DP_CPM_C;//差压温度补偿 数据库读取
	float 	DP_CPM_D;//差压绝压补偿 数据库读取

	float	TF_PCPM;//工况华式摄氏度 计算得到
	float	TF_PCPM_SC;//标况华式摄氏度 数据库读取
	float	LS_P_CPM;//工况压力 计算得到
	float	LS_DP_CPM;//工况差压 计算得到
	float	P_CPM_SC;//标况压力 数据库读取
	float	F_CPM;//356/31比值 数据库读取
	float	D_CPM;//管直径，喉颈位置 数据库读取
	float	G31_CPM;//气体31吸收系数 计算获得
	float	G31_CPM_A;
	float	G31_CPM_B;
	float	W31_CPM;//满水31吸收系数 数据库读取
	float	G356_CPM;//气体356吸收系数 计算获得
	float	G356_CPM_A;
	float	G356_CPM_B;
	float	W356_CPM;//满水356吸收系数 数据库读取
	float	OVF_CPM;//体积含水率 计算得到
	float	GVF_CPM;//体积含气率 计算得到
	float	OMF_CPM;//质量含水率 计算得到
	float	GMF_CPM;//质量含气率 计算得到
	float	GD_CPM;//气密度LC 工况；计算得到
	float	WD_CPM;//水密度LC 工况；数据库读取
	float	GD_CPM_SC;//气密度SC 标况；数据库读取

	float	Pmix_CPM;//混合密度 计算得到
	float	b_CPM;//贝塔值 数据库读取
	float	C_CPM;//流出系数 数据库读取
	float	E_CPM;//流束膨胀系数 数据库读取
	float	x1;
	float	x2;
	float	Qm_CPM;//计算得到
	float	Qv_CPM;//计算得到

	//修正参数
	float	OVF_CPM_A;
	float	OVF_CPM_B;
	float	OMF_CPM_A;
	float	OMF_CPM_B;
	float	Qv_CPM_A;
	float	Qv_CPM_B;
	float	Qm_CPM_A;
	float	Qm_CPM_B;

	OVF_CPM_A	=	corrs.OVF_CPM_A;
	OVF_CPM_B	=	corrs.OVF_CPM_B;
	OMF_CPM_A	=	corrs.OMF_CPM_A;
	OMF_CPM_B	=	corrs.OMF_CPM_B;
	Qv_CPM_A	=	corrs.Qv_CPM_A;
	Qv_CPM_B	=	corrs.Qv_CPM_B;
	Qm_CPM_A	=	corrs.Qm_CPM_A;
	Qm_CPM_B	=	corrs.Qm_CPM_B;


	k31_CPM		=	flowresult.winB;//
	k81_CPM		=	flowresult.winC;
	k356_CPM	=	flowresult.winE;
	DP_CPM		=	shared->ad1115[0][2];
	P_CPM		=	shared->ad1115[0][1];
	T_P_CPM		=	shared->ad1115[0][0];
	T_DP_CPM	=	shared->DEV_T2;

	F_CPM		=	cpms.F_CPM;//1.549025;
	D_CPM		=	cpms.D_CPM;//0.0254;
	G31_CPM_A	=	cpms.G31_CPM_A;//0.006;
	G31_CPM_B	=	cpms.G31_CPM_B;//-1.612;
	G356_CPM_A	=	cpms.G356_CPM_A;//0.006;
	G356_CPM_B	=	cpms.G356_CPM_B;//-0.1574;
	G31_CPM		=	G31_CPM_A*P_CPM+G31_CPM_B;
	G356_CPM	=	G356_CPM_A*P_CPM+G356_CPM_B;
	W31_CPM		=	cpms.W31_CPM;//28.68686;
	W356_CPM	=	cpms.W356_CPM;//10.52329;
	GD_CPM_SC	=	cpms.GD_CPM_SC;//1.293;
	TF_PCPM_SC	=	cpms.TF_PCPM_SC;//273.15;
	P_CPM_SC	=	cpms.P_CPM_SC;//101;kPa
	P_CPM_A		=	cpms.P_CPM_A;//42.95;
	P_CPM_B		=	cpms.P_CPM_B;//-11787.4;
	DP_CPM_A	=	cpms.DP_CPM_A;//5.128205;
	DP_CPM_C	=	cpms.DP_CPM_C;//2.28205;
	DP_CPM_D	=	cpms.DP_CPM_D;//-0.139887;
	DP_CPM_B	=	cpms.DP_CPM_B;//-191.14102;

	LS_P_CPM	=	P_CPM_A*P_CPM+P_CPM_B;//工况压力修正公式
	shared->DataP	=	LS_P_CPM;
	TF_PCPM		=	T_P_CPM+273.15;//工况华式温度

	WD_CPM		=	cpms.WD_CPM;
	GD_CPM		=	GD_CPM_SC*TF_PCPM_SC*LS_P_CPM/(P_CPM_SC*TF_PCPM);//工况气密度

	OVF_CPM		=	((log(k356_CPM/(F_CPM*k31_CPM))-D_CPM*(G31_CPM-G356_CPM))/(W31_CPM-W356_CPM-G31_CPM+G356_CPM))/D_CPM;
	OVF_CPM		=	OVF_CPM_A*OVF_CPM+OVF_CPM_B;
	if(OVF_CPM>0&&OVF_CPM<1){
		GVF_CPM		=	1-OVF_CPM;
	}
	else if(OVF_CPM<0){
		OVF_CPM	=	0;
		GVF_CPM	=	1;
	}
	else if(OVF_CPM>1){
		OVF_CPM	=	1;
		GVF_CPM	=	0;
	}
	else{
		OVF_CPM	=	0;
		GVF_CPM	=	0;
	}

	OMF_CPM		=	OVF_CPM*WD_CPM/(OVF_CPM*WD_CPM+GVF_CPM*GD_CPM);
	OMF_CPM		=	OMF_CPM_A*OMF_CPM+OMF_CPM_B;
	printf("OMF=%f\n",OMF_CPM);
	if(OMF_CPM>0&&OMF_CPM<1){
		GMF_CPM		=	1-OMF_CPM;
	}
	else if(OMF_CPM<0){
		OMF_CPM	=	0;
		GMF_CPM	=	1;
	}
	else if(OMF_CPM>1){
		OMF_CPM	=	1;
		GMF_CPM	=	0;
	}
	else{
		OMF_CPM	=	0;
		GMF_CPM	=	0;
	}
	/*
	 * 相分计算完毕，后面计算总流量
	 */
	b_CPM			=	cpms.b_CPM;//0.5;
	C_CPM			=	cpms.C_CPM;//0.995;
	E_CPM			=	cpms.E_CPM;//1;
	LS_DP_CPM		=	DP_CPM_A*DP_CPM	+	DP_CPM_C*T_DP_CPM	+	DP_CPM_D*P_CPM	+	DP_CPM_B;//pa
	shared->DataDP	=	LS_DP_CPM;
	Pmix_CPM		=	WD_CPM*OVF_CPM	+	GD_CPM*GVF_CPM;
	x1				=	1-pow(b_CPM,4);
	x2				=	LS_DP_CPM*Pmix_CPM*2;
	if(x1<0||x1==0)
	{
		x1=1;
		printf("error\n");
	}
	if(x2<0)
	{
		x2=0;
	}
	Qm_CPM			=	C_CPM/pow(x1,0.5)*E_CPM*(M_PI/4)*pow(D_CPM,2)*pow(x2,0.5);
	Qv_CPM			=	Qm_CPM/Pmix_CPM;
	Qm_CPM			=	Qm_CPM_A*Qm_CPM	+	Qm_CPM_B;
	Qv_CPM			=	Qv_CPM_A*Qv_CPM	+	Qv_CPM_B;

	//结果输出
	cpms.OVF_CPM	=	OVF_CPM;
	cpms.GVF_CPM	=	GVF_CPM;
	cpms.Qv_CPM		=	Qv_CPM;
	cpms.OMF_CPM	=	OMF_CPM;
	cpms.GMF_CPM	=	GMF_CPM;
	cpms.Qm_CPM		=	Qm_CPM;

	shared->OMF_CPM =	OMF_CPM;
	shared->GMF_CPM	=	GMF_CPM;
	shared->Qm_CPM	=	Qm_CPM;

}
/**********10.1更改***************/
void CPM1(void){

	int i;
	//计算原始量6个
	int 	k31_CPM;//31能量计数每分钟 硬件获取
	int 	K31_CPM_Empty;//k31每分钟空管计数
	int 	k81_CPM;//81能量计数每分钟 硬件获取
	int 	k356_CPM;//356能量计数每分钟 硬件获取
	float 	DP_CPM;//差压数值 硬件获取
	float 	P_CPM;//绝压数值 硬件获取
	float 	T_P_CPM;//绝压温度 硬件获取
	float 	T_DP_CPM;//差压温度 硬件获取

	float 	P_CPM_A;//绝压数值修正系数 数据库读取
	float 	P_CPM_B;//绝压数值修正系数 数据库读取
	float 	DP_CPM_A;//差压数值修正系数 数据库读取
	float 	DP_CPM_B;//差压数值修正系数 数据库读取
	float 	DP_CPM_C;//差压温度补偿 数据库读取
	float 	DP_CPM_D;//差压绝压补偿 数据库读取

	float	TF_PCPM;//工况华式摄氏度 计算得到
	float	TF_PCPM_SC;//标况华式摄氏度 数据库读取
	float	LS_P_CPM;//工况压力 计算得到
	float	LS_DP_CPM;//工况差压 计算得到
	float	P_CPM_SC;//标况压力 数据库读取
	float	F_CPM;//356/31比值 数据库读取
	float	D_CPM;//管直径，喉颈位置 数据库读取
	float	G31_CPM;//气体31吸收系数 计算获得
	float	G31_CPM_A;
	float	G31_CPM_B;
	float	W31_CPM;//满水31吸收系数 数据库读取
	float	G356_CPM;//气体356吸收系数 计算获得
	float	G356_CPM_A;
	float	G356_CPM_B;
	float	W356_CPM;//满水356吸收系数 数据库读取
	float	OVF_CPM;//体积含水率 计算得到
	float	GVF_CPM;//体积含气率 计算得到
	float	OMF_CPM;//质量含水率 计算得到
	float	GMF_CPM;//质量含气率 计算得到
	float	GD_CPM;//气密度LC 工况；计算得到
	float	WD_CPM;//水密度LC 工况；数据库读取
	float	GD_CPM_SC;//气密度SC 标况；数据库读取

	float	Pmix_CPM;//混合密度 计算得到
	float	b_CPM;//贝塔值 数据库读取
	float	C_CPM;//流出系数 数据库读取
	float	E_CPM;//流束膨胀系数 数据库读取
	float	x1;
	float	x2;
	float	Qm_CPM = 0;//计算得到
	float	Qv_CPM = 0;//计算得到
	float  ovf1;//计算中间量
	float  ovf2;//计算中间量
	float  ovf3;
	//平均
	static int CPM_NO = 1;
	static float	OVF_CPM_Ave[21] = {0};
	static float	GVF_CPM_Ave[21] = {0};
	static float	Qv_CPM_Ave[21] = {0};
	static float	OMF_CPM_Ave[21] = {0};
	static float	GMF_CPM_Ave[21] = {0};
	static float	Qm_CPM_Ave[21] = {0};
	//修正参数
	float	OVF_CPM_A;
	float	OVF_CPM_B;
	float	OMF_CPM_A;
	float	OMF_CPM_B;
	float	Qv_CPM_A;
	float	Qv_CPM_B;
	float	Qm_CPM_A;
	float	Qm_CPM_B;

	OVF_CPM_A	=	corrs.OVF_CPM_A;
	OVF_CPM_B	=	corrs.OVF_CPM_B;
	OMF_CPM_A	=	corrs.OMF_CPM_A;
	OMF_CPM_B	=	corrs.OMF_CPM_B;
	Qv_CPM_A	=	corrs.Qv_CPM_A;
	Qv_CPM_B	=	corrs.Qv_CPM_B;
	Qm_CPM_A	=	corrs.Qm_CPM_A;
	Qm_CPM_B	=	corrs.Qm_CPM_B;
	K31_CPM_Empty = corrs.K31_Empty;

	k31_CPM		=	flowresult.winB;//k31能量计数
	k81_CPM		=	flowresult.winC;//k81能量计数
	k356_CPM	=	flowresult.winE;//356能量计数 （无用）
	DP_CPM		=	shared->ad1115[0][2];//差压 （添加门限，门限以下均为0）
	P_CPM		=	shared->ad1115[0][1];//绝压
	T_P_CPM		=	shared->ad1115[0][0];//绝压温度
	T_DP_CPM	=	shared->DEV_T2;//差压温度

	F_CPM		=	cpms.F_CPM;//1.549025;
	D_CPM		=	cpms.D_CPM;//0.0254;
	G31_CPM_A	=	cpms.G31_CPM_A;//0.006;//
	G31_CPM_B	=	cpms.G31_CPM_B;//-1.612;
	G356_CPM_A	=	cpms.G356_CPM_A;//0.006;
	G356_CPM_B	=	cpms.G356_CPM_B;//-0.1574;

	W31_CPM		=	cpms.W31_CPM;//28.68686;
	W356_CPM	=	cpms.W356_CPM;//10.52329;
	GD_CPM_SC	=	cpms.GD_CPM_SC;//1.293;
	TF_PCPM_SC	=	cpms.TF_PCPM_SC;//273.15;
	P_CPM_SC	=	cpms.P_CPM_SC;//101;kPa
	P_CPM_A		=	cpms.P_CPM_A;//42.95;
	P_CPM_B		=	cpms.P_CPM_B;//-11787.4;
	DP_CPM_A	=	cpms.DP_CPM_A;//5.128205;
	DP_CPM_C	=	cpms.DP_CPM_C;//2.28205;
	DP_CPM_D	=	cpms.DP_CPM_D;//-0.139887;
	DP_CPM_B	=	cpms.DP_CPM_B;//-191.14102;

	LS_P_CPM	=	P_CPM_A*P_CPM+P_CPM_B;//工况压力修正公式
	printf("LS_P_CPM	=	P_CPM_A*P_CPM+P_CPM_B\n");
	printf("LS_P_CPM = %f P_CPM_A = %f P_CPM = %f P_CPM_B = %f\n",LS_P_CPM, P_CPM_A, P_CPM, P_CPM_B);
	G31_CPM		=	G31_CPM_A*LS_P_CPM+G31_CPM_B;
	printf("G31_CPM = G31_CPM_A*LS_P_CPM+G31_CPM_B\n");
	printf("G31_CPM = %f G31_CPM_A = %f LS_P_CPM = %f G31_CPM_B = %f\n",G31_CPM, G31_CPM_A, LS_P_CPM, G31_CPM_B);
	G356_CPM	=	G356_CPM_A*LS_P_CPM+G356_CPM_B;
	printf("G356_CPM	=	G356_CPM_A*LS_P_CPM+G356_CPM_B\n");
	printf("G356_CPM = %f G356_CPM_A = %f LS_P_CPM = %f G356_CPM_B = %f\n",G356_CPM, G356_CPM_A, LS_P_CPM, G356_CPM_B);
	shared->DataP	=	LS_P_CPM;
	TF_PCPM		=	T_P_CPM+273.15;//工况华式温度

	WD_CPM		=	cpms.WD_CPM;
	GD_CPM		=	GD_CPM_SC*TF_PCPM_SC*LS_P_CPM/(P_CPM_SC*TF_PCPM);//工况气密度
	printf("GD_CPM		=	GD_CPM_SC*TF_PCPM_SC*LS_P_CPM/(P_CPM_SC*TF_PCPM)\n");
	printf("GD_CPM = %f GD_CPM_SC = %f TF_PCPM_SC = %f LS_P_CPM = %f P_CPM_SC= %f TF_PCPM = %f\n",GD_CPM, GD_CPM_SC, TF_PCPM_SC, LS_P_CPM, P_CPM_SC, TF_PCPM);
	//OVF_CPM		=	((log(k356_CPM/(F_CPM*k31_CPM))-D_CPM*(G31_CPM-G356_CPM))/(W31_CPM-W356_CPM-G31_CPM+G356_CPM))/D_CPM;
	//OVF_CPM		=	OVF_CPM_A*OVF_CPM+OVF_CPM_B;
#if 0
	K31_CPM_Empty = 154133;
	k31_CPM = 153705;
	D_CPM = 0.0254;
	G31_CPM = -0.037482;
	W31_CPM = 25.382364;
#endif
/*	ovf1 = (float)K31_CPM_Empty / (float)k31_CPM;
	printf("ovf1 = %f\n",ovf1);
	ovf1 = log(ovf1);
	printf("ovf1 = %f\n",ovf1);
	ovf2 = (ovf1- D_CPM * G31_CPM) / (W31_CPM - G31_CPM);
	printf("ovf2 = %f\n",ovf2);
	OVF_CPM		=	ovf2 / D_CPM;*/
	OVF_CPM		=	(((log((float)K31_CPM_Empty / (float)k31_CPM) / log(exp(1))) - D_CPM * G31_CPM) / (W31_CPM - G31_CPM)) / D_CPM;

	printf("	OVF_CPM		=	(((log(K31_CPM_Empty / k31_CPM) / log(exp(1))) - D_CPM * G31_CPM) / (W31_CPM - G31_CPM)) / D_CPM \n");
	printf("OVF_CPM = %f K31_CPM_Empty = %d k31_CPM = %d D_CPM = %f G31_CPM= %f W31_CPM = %f\n",OVF_CPM, K31_CPM_Empty, k31_CPM, D_CPM, G31_CPM, W31_CPM);
	if(OVF_CPM >= 0.0 && OVF_CPM <= 1.0){
		GVF_CPM		=	1-OVF_CPM;
	}
	else if(OVF_CPM < 0.0){
		OVF_CPM	=	0;
		GVF_CPM	=	1;
	}
	else if(OVF_CPM > 1.0){
		OVF_CPM	=	1.0;
		GVF_CPM	=	0.0;
	}


	OMF_CPM		=	OVF_CPM*WD_CPM/(OVF_CPM*WD_CPM+GVF_CPM*GD_CPM);
	printf("	OMF_CPM		=	OVF_CPM*WD_CPM/(OVF_CPM*WD_CPM+GVF_CPM*GD_CPM) \n");
	printf("OMF_CPM = %f OVF_CPM = %f WD_CPM = %f GVF_CPM = %f GD_CPM= %f \n",OMF_CPM, OVF_CPM, WD_CPM, GVF_CPM, GD_CPM);
	OMF_CPM		=	OMF_CPM_A*OMF_CPM+OMF_CPM_B;
	printf("		OMF_CPM		=	OMF_CPM_A*OMF_CPM+OMF_CPM_B; \n");
	printf("OMF_CPM = %f OMF_CPM_A = %f OMF_CPM_B = %f \n",OMF_CPM, OMF_CPM_A, OMF_CPM_B);

	if(OMF_CPM >= 0.0 && OMF_CPM <= 1.0){
		GMF_CPM		=	1.0-OMF_CPM;
	}
	else if(OMF_CPM < 0.0){
		OMF_CPM	=	0.0;
		GMF_CPM	=	1.0;
	}
	else if(OMF_CPM>1){
		OMF_CPM	=	1.0;
		GMF_CPM	=	0.0;
	}

	/*
	 * 相分计算完毕，后面计算总流量
	 */
	b_CPM			=	cpms.b_CPM;//0.5;
	C_CPM			=	cpms.C_CPM;//0.995;
	E_CPM			=	cpms.E_CPM;//1;
	LS_DP_CPM		=	DP_CPM_A*DP_CPM	+	DP_CPM_C*T_DP_CPM	+	DP_CPM_D*P_CPM	+	DP_CPM_B;//pa
	printf("LS_DP_CPM		=	DP_CPM_A*DP_CPM	+	DP_CPM_C*T_DP_CPM	+	DP_CPM_D*P_CPM	+	DP_CPM_B \n");
	printf("LS_DP_CPM = %f DP_CPM_A = %f DP_CPM = %f DP_CPM_C=%f T_DP_CPM=%f\n",LS_DP_CPM, DP_CPM_A, DP_CPM,DP_CPM_C,T_DP_CPM);
	printf("DP_CPM_D = %f P_CPM = %f DP_CPM_B = %f\n",DP_CPM_D, P_CPM, DP_CPM_B);

	printf("LS_DP_CPM =%f corrs.DP_th =%f\n",LS_DP_CPM, corrs.DP_th);
	if(LS_DP_CPM < corrs.DP_th){
		LS_DP_CPM = 0;
	}
	shared->DataDP	=	LS_DP_CPM;
	Pmix_CPM		=	WD_CPM*OVF_CPM	+	GD_CPM*GVF_CPM;
	printf("Pmix_CPM		=	WD_CPM*OVF_CPM	+	GD_CPM*GVF_CPM \n");
	printf("Pmix_CPM = %f WD_CPM = %f OVF_CPM = %f GD_CPM=%f GVF_CPM=%f\n",Pmix_CPM, WD_CPM, OVF_CPM,GD_CPM,GVF_CPM);
	x1				=	1-pow(b_CPM,4);
	x2				=	LS_DP_CPM*Pmix_CPM*2;
	if(x1<0||x1==0)
	{
		x1=1;
		printf("error\n");
	}
	if(x2<0)
	{
		x2=0;
	}
	//Qm_CPM			=	C_CPM/pow(x1,0.5)*E_CPM*(M_PI/4)*pow(E_CPM,2)*pow(x2,0.5);
	//Qv_CPM			=	Qm_CPM/Pmix_CPM;
	//Qm_CPM			=	Qm_CPM_A*Qm_CPM	+	Qm_CPM_B;
	//Qv_CPM			=	Qv_CPM_A*Qv_CPM	+	Qv_CPM_B;
printf("LS_DP_CPM =%f\n",LS_DP_CPM);
	Qv_CPM			=	0.004 * C_CPM * E_CPM * D_CPM * D_CPM * pow((LS_DP_CPM / Pmix_CPM), 0.5);
printf("0.004 * C_CPM * E_CPM * D_CPM * D_CPM * pow((LS_DP_CPM / Pmix_CPM), 0.5)");
printf("C_CPM = %f E_CPM = %f D_CPM = %f Pmix_CPM = %f",C_CPM,E_CPM,D_CPM,Pmix_CPM);
	Qm_CPM			=	Qv_CPM*Pmix_CPM;
	Qm_CPM			=	Qm_CPM_A*Qm_CPM	+	Qm_CPM_B;
	Qv_CPM			=	Qv_CPM_A*Qv_CPM	+	Qv_CPM_B;
#if 0 //
	cpms.OVF_CPM	=	OVF_CPM;
	cpms.GVF_CPM	=	GVF_CPM;
	cpms.Qv_CPM		=	Qv_CPM;
	cpms.OMF_CPM	=	OMF_CPM;
	cpms.GMF_CPM	=	GMF_CPM;
	cpms.Qm_CPM		=	Qm_CPM;

	shared->OMF_CPM =	cpms.OMF_CPM;
	shared->GMF_CPM	=	cpms.GMF_CPM;
	shared->Qm_CPM	=	cpms.Qm_CPM;

	shared->Qm_SUM	=	shared->Qm_CPM	+	shared->Qm_SUM;
#endif

#if 1
	OVF_CPM_Ave[CPM_NO] = OVF_CPM;
	GVF_CPM_Ave[CPM_NO] = GVF_CPM;
	Qv_CPM_Ave[CPM_NO] = Qv_CPM;
	OMF_CPM_Ave[CPM_NO] = OMF_CPM;
	GMF_CPM_Ave[CPM_NO] = GMF_CPM;
	Qm_CPM_Ave[CPM_NO] = Qm_CPM;
#endif
#if 1
printf("CPM_NO = %d\n",CPM_NO);
printf("$$$$$$$$$$$$$$$$$$$$$$\n");
printf("OVF_CPM_Ave[0] = %f OVF_CPM_Ave[1] = %f OVF_CPM_Ave[2] = %f OVF_CPM_Ave[3] = %f OVF_CPM_Ave[4] = %f\n",OVF_CPM_Ave[0],
		OVF_CPM_Ave[1], OVF_CPM_Ave[2], OVF_CPM_Ave[3], OVF_CPM_Ave[4]);
printf("cpms.OVF_CPM = %f, corrs.CMP_ave = %f\n",cpms.OVF_CPM, corrs.CMP_ave);
printf("$$$$$$$$$$$$$$$$$$$$$$\n");
#endif
#if 1
	if (CPM_NO >= (int)corrs.CMP_ave)
	{
	//结果输出
		for (i = 0; i < (int)corrs.CMP_ave; i++)
		{
			OVF_CPM_Ave[i] = OVF_CPM_Ave[i+1];
			GVF_CPM_Ave[i] = GVF_CPM_Ave[i+1];
			Qv_CPM_Ave[i] = Qv_CPM_Ave[i+1];
			OMF_CPM_Ave[i] = OMF_CPM_Ave[i+1];
			GMF_CPM_Ave[i] = GMF_CPM_Ave[i+1];
			Qm_CPM_Ave[i] = Qm_CPM_Ave[i+1];
		}
		cpms.OVF_CPM	=	0;
		cpms.GVF_CPM	=	0;
		cpms.Qv_CPM		=	0;
		cpms.OMF_CPM	=	0;
		cpms.GMF_CPM	=	0;
		cpms.Qm_CPM		=	0;
		for (i = 0; i < (int)corrs.CMP_ave; i++)
		{
			cpms.OVF_CPM	+=	OVF_CPM_Ave[i];
			cpms.GVF_CPM	+=	GVF_CPM_Ave[i];
			cpms.Qv_CPM		+=	Qv_CPM_Ave[i];
			cpms.OMF_CPM	+=	OMF_CPM_Ave[i];
			cpms.GMF_CPM	+=	GMF_CPM_Ave[i];
			cpms.Qm_CPM		+=	Qm_CPM_Ave[i];
		}
		printf("OVF_CPM_Ave[0] = %f OVF_CPM_Ave[1] = %f OVF_CPM_Ave[2] = %f OVF_CPM_Ave[3] = %f OVF_CPM_Ave[4] = %f\n",OVF_CPM_Ave[0],
				OVF_CPM_Ave[1], OVF_CPM_Ave[2], OVF_CPM_Ave[3], OVF_CPM_Ave[4]);
		printf("cpms.OVF_CPM = %f, corrs.CMP_ave = %f\n",cpms.OVF_CPM, corrs.CMP_ave);
		cpms.OVF_CPM	=	cpms.OVF_CPM / corrs.CMP_ave;
		cpms.GVF_CPM	=	cpms.GVF_CPM / corrs.CMP_ave;
		cpms.Qv_CPM		=	cpms.Qv_CPM / corrs.CMP_ave;
		cpms.OMF_CPM	=	cpms.OMF_CPM / corrs.CMP_ave;
		cpms.GMF_CPM	=	cpms.GMF_CPM / corrs.CMP_ave;
		cpms.Qm_CPM		=	cpms.Qm_CPM / corrs.CMP_ave;
#if 0 //每秒显示改为每分钟显示这里写1 名注视掉CPS
		shared->OMF_CPM =	cpms.OMF_CPM;
		shared->GMF_CPM	=	cpms.GMF_CPM;
		shared->Qm_CPM	=	cpms.Qm_CPM;

		shared->Qm_SUM	=	shared->Qm_CPM	+	shared->Qm_SUM;



#endif
	}
	else{
		CPM_NO++;
	}
#endif
	if(LS_DP_CPM == 0){
		cpms.OVF_CPM	=	0;
		cpms.GVF_CPM	=	0;
		cpms.OMF_CPM	=	0;
		cpms.GMF_CPM	=	0;
		shared->OMF_CPM =	0;
		shared->GMF_CPM	=	0;
	}
}
/*11.15 秒计算*/
void CPS(void){
	float	b_CPM;//贝塔值 数据库读取
	float	C_CPM;//流出系数 数据库读取
	float	E_CPM;//流束膨胀系数 数据库读取
	float	LS_DP_CPM;//工况差压 计算得到
	float 	DP_CPM_A;//差压数值修正系数 数据库读取
	float 	DP_CPM_B;//差压数值修正系数 数据库读取
	float 	DP_CPM_C;//差压温度补偿 数据库读取
	float 	DP_CPM_D;//差压绝压补偿 数据库读取
	float 	DP_CPM;//差压数值 硬件获取
	float 	P_CPM;//绝压数值 硬件获取
	float 	T_P_CPM;//绝压温度 硬件获取
	float 	T_DP_CPM;//差压温度 硬件获取
	float	Pmix_CPM;//混合密度 计算得到
	float	GD_CPM;//气密度LC 工况；计算得到
	float	WD_CPM;//水密度LC 工况；数据库读取
	float	GD_CPM_SC;
	float	TF_PCPM_SC;
	float	LS_P_CPM;
	float	P_CPM_A;
	float	P_CPM_B;
	float	P_CPM_SC;
	float	TF_PCPM;
	float	x1;
	float	x2;
	float	Qm_CPM = 0;//计算得到
	float	Qv_CPM = 0;//计算得到
	float	D_CPM;
	float	Qm_CPM_A;
	float	Qm_CPM_B;
	float	Qml_CPM = 0;//计算得到
	float	Qmg_CPM = 0;//计算得到

	Qm_CPM_A	=	corrs.Qm_CPM_A;
	Qm_CPM_B	=	corrs.Qm_CPM_B;
	D_CPM		=	cpms.D_CPM;//0.0254;
	WD_CPM		=	cpms.WD_CPM;
	DP_CPM		=	shared->ad1115[0][2];//差压 （添加门限，门限以下均为0）
	P_CPM		=	shared->ad1115[0][1];//绝压
	T_P_CPM		=	shared->ad1115[0][0];//绝压温度
	T_DP_CPM	=	shared->DEV_T2;//差压温度

	DP_CPM_A	=	cpms.DP_CPM_A;//5.128205;
	DP_CPM_C	=	cpms.DP_CPM_C;//2.28205;
	DP_CPM_D	=	cpms.DP_CPM_D;//-0.139887;
	DP_CPM_B	=	cpms.DP_CPM_B;//-191.14102;
	P_CPM_A		=	cpms.P_CPM_A;//42.95;
	P_CPM_B		=	cpms.P_CPM_B;//-11787.4;
	b_CPM		=	cpms.b_CPM;//0.5;
	C_CPM		=	cpms.C_CPM;//0.995;
	E_CPM		=	cpms.E_CPM;//1;

	LS_P_CPM	=	P_CPM_A*P_CPM+P_CPM_B;//工况压力修正公式
	TF_PCPM_SC	=	cpms.TF_PCPM_SC;//273.15;
	GD_CPM_SC	=	cpms.GD_CPM_SC;//1.293;
	P_CPM_SC	=	cpms.P_CPM_SC;//101;kPa
	TF_PCPM		=	T_P_CPM+273.15;//工况华式温度
	GD_CPM		=	GD_CPM_SC*TF_PCPM_SC*LS_P_CPM/(P_CPM_SC*TF_PCPM);//工况气密度

	LS_DP_CPM		=	DP_CPM_A*DP_CPM	+	DP_CPM_C*T_DP_CPM	+	DP_CPM_D*P_CPM	+	DP_CPM_B;//pa
	printf("1s:LS_DP_CPM		=	DP_CPM_A*DP_CPM	+	DP_CPM_C*T_DP_CPM	+	DP_CPM_D*P_CPM	+	DP_CPM_B \n");
	printf("1s:LS_DP_CPM = %f DP_CPM_A = %f DP_CPM = %f DP_CPM_C=%f T_DP_CPM=%f\n",LS_DP_CPM, DP_CPM_A, DP_CPM,DP_CPM_C,T_DP_CPM);
	printf("1s:DP_CPM_D = %f P_CPM = %f DP_CPM_B = %f\n",DP_CPM_D, P_CPM, DP_CPM_B);
	printf("1s:LS_DP_CPM =%f corrs.DP_th =%f\n",LS_DP_CPM, corrs.DP_th);
	if(LS_DP_CPM < corrs.DP_th){
		LS_DP_CPM = 0;
	}
	Pmix_CPM		=	WD_CPM*cpms.OVF_CPM	+	GD_CPM*cpms.GVF_CPM;
	printf("1s:Pmix_CPM		=	WD_CPM*OVF_CPM	+	GD_CPM*GVF_CPM \n");
	printf("1s:Pmix_CPM = %f WD_CPM = %f OVF_CPM = %f GD_CPM=%f GVF_CPM=%f\n",Pmix_CPM, WD_CPM, cpms.OVF_CPM	,GD_CPM,cpms.GVF_CPM);
	x1				=	1-pow(b_CPM,4);
	x2				=	LS_DP_CPM*Pmix_CPM*2;
	if(x1<0||x1==0)
	{
		x1=1;
		printf("error\n");
	}
	if(x2<0)
	{
		x2=0;
	}
	//LS_DP_CPM = 100;
	if (Pmix_CPM == 0)
	{
		Pmix_CPM = 1;
	}
	if (LS_DP_CPM < 0)
	{
		LS_DP_CPM = 0;
	}
	printf("1s:LS_DP_CPM =%f\n",LS_DP_CPM);
	Qv_CPM			=	0.004 * C_CPM * E_CPM * D_CPM * D_CPM * pow((LS_DP_CPM / Pmix_CPM), 0.5);
	printf("1s:Qv_CPM =%f\n",Qv_CPM);
	Qm_CPM			=	Qv_CPM*Pmix_CPM;
	Qm_CPM			=	Qm_CPM_A*Qm_CPM	+	Qm_CPM_B;
	printf("1s:Qm_CPM =%f cpms.OMF_CPM =%f cpms.GMF_CPM =%f\n",Qm_CPM,cpms.OMF_CPM,cpms.GMF_CPM);

	Qml_CPM			=	(Qm_CPM * cpms.OMF_CPM) / 60.0;
	Qmg_CPM			=	(Qm_CPM * cpms.GMF_CPM) / 60.0;


    shared->hour	=	m_time.hour;
    shared->minute	=	m_time.minute;
    shared->second	=	m_time.second;
    shared->Qml_CPM	=	Qml_CPM + shared->Qml_CPM;
    shared->Qmg_CPM	=	Qmg_CPM + shared->Qmg_CPM;
    printf("shared->Qml_CPM	 = %f shared->Qmg_CPM = %f\n",shared->Qml_CPM,shared->Qmg_CPM);

}
/**********10.1更改***************/
void CPM2(void){

	int i;
	//计算原始量6个
	int 	k31_CPM;//31能量计数每分钟 硬件获取
	int 	K31_CPM_Empty;//k31每分钟空管计数
	int 	k81_CPM;//81能量计数每分钟 硬件获取
	int 	k356_CPM;//356能量计数每分钟 硬件获取
	float 	DP_CPM;//差压数值 硬件获取
	float 	P_CPM;//绝压数值 硬件获取
	float 	T_P_CPM;//绝压温度 硬件获取
	float 	T_DP_CPM;//差压温度 硬件获取

	float 	P_CPM_A;//绝压数值修正系数 数据库读取
	float 	P_CPM_B;//绝压数值修正系数 数据库读取
	float 	DP_CPM_A;//差压数值修正系数 数据库读取
	float 	DP_CPM_B;//差压数值修正系数 数据库读取
	float 	DP_CPM_C;//差压温度补偿 数据库读取
	float 	DP_CPM_D;//差压绝压补偿 数据库读取

	float	TF_PCPM;//工况华式摄氏度 计算得到
	float	TF_PCPM_SC;//标况华式摄氏度 数据库读取
	float	LS_P_CPM;//工况压力 计算得到
	float	LS_DP_CPM;//工况差压 计算得到
	float	P_CPM_SC;//标况压力 数据库读取
	float	F_CPM;//356/31比值 数据库读取
	float	D_CPM;//管直径，喉颈位置 数据库读取
	float	G31_CPM;//气体31吸收系数 计算获得
	float	G31_CPM_A;
	float	G31_CPM_B;
	float	W31_CPM;//满水31吸收系数 数据库读取
	float	G356_CPM;//气体356吸收系数 计算获得
	float	G356_CPM_A;
	float	G356_CPM_B;
	float	W356_CPM;//满水356吸收系数 数据库读取
	float	OVF_CPM;//体积含水率 计算得到
	float	GVF_CPM;//体积含气率 计算得到
	float	OMF_CPM;//质量含水率 计算得到
	float	GMF_CPM;//质量含气率 计算得到
	float	GD_CPM;//气密度LC 工况；计算得到
	float	WD_CPM;//水密度LC 工况；数据库读取
	float	GD_CPM_SC;//气密度SC 标况；数据库读取

	float	Pmix_CPM;//混合密度 计算得到
	float	b_CPM;//贝塔值 数据库读取
	float	C_CPM;//流出系数 数据库读取
	float	E_CPM;//流束膨胀系数 数据库读取
	float	x1;
	float	x2;
	float	Qm_CPM = 0;//计算得到
	float	Qv_CPM = 0;//计算得到
	float  ovf1;//计算中间量
	float  ovf2;//计算中间量
	float  ovf3;
	//平均
	static int CPM_NO = 1;
	static float	OVF_CPM_Ave[21] = {0};
	static float	GVF_CPM_Ave[21] = {0};
	static float	Qv_CPM_Ave[21] = {0};
	static float	OMF_CPM_Ave[21] = {0};
	static float	GMF_CPM_Ave[21] = {0};
	static float	Qm_CPM_Ave[21] = {0};
	//修正参数
	float	OVF_CPM_A;
	float	OVF_CPM_B;
	float	OMF_CPM_A;
	float	OMF_CPM_B;
	float	Qv_CPM_A;
	float	Qv_CPM_B;
	float	Qm_CPM_A;
	float	Qm_CPM_B;

	OVF_CPM_A	=	corrs.OVF_CPM_A;
	OVF_CPM_B	=	corrs.OVF_CPM_B;
	OMF_CPM_A	=	corrs.OMF_CPM_A;
	OMF_CPM_B	=	corrs.OMF_CPM_B;
	Qv_CPM_A	=	corrs.Qv_CPM_A;
	Qv_CPM_B	=	corrs.Qv_CPM_B;
	Qm_CPM_A	=	corrs.Qm_CPM_A;
	Qm_CPM_B	=	corrs.Qm_CPM_B;
	K31_CPM_Empty = corrs.K31_Empty;

	k31_CPM		=	flowresult.winB;//k31能量计数
	k81_CPM		=	flowresult.winC;//k81能量计数
	k356_CPM	=	flowresult.winE;//356能量计数 （无用）
	DP_CPM		=	shared->ad1115[0][2];//差压 （添加门限，门限以下均为0）
	P_CPM		=	shared->ad1115[0][1];//绝压
	T_P_CPM		=	shared->ad1115[0][0];//绝压温度
	T_DP_CPM	=	shared->DEV_T2;//差压温度

	F_CPM		=	cpms.F_CPM;//1.549025;
	D_CPM		=	cpms.D_CPM;//0.0254;
	G31_CPM_A	=	cpms.G31_CPM_A;//0.006;//
	G31_CPM_B	=	cpms.G31_CPM_B;//-1.612;
	G356_CPM_A	=	cpms.G356_CPM_A;//0.006;
	G356_CPM_B	=	cpms.G356_CPM_B;//-0.1574;

	W31_CPM		=	cpms.W31_CPM;//28.68686;
	W356_CPM	=	cpms.W356_CPM;//10.52329;
	GD_CPM_SC	=	cpms.GD_CPM_SC;//1.293;
	TF_PCPM_SC	=	cpms.TF_PCPM_SC;//273.15;
	P_CPM_SC	=	cpms.P_CPM_SC;//101;kPa
	P_CPM_A		=	cpms.P_CPM_A;//42.95;
	P_CPM_B		=	cpms.P_CPM_B;//-11787.4;
	DP_CPM_A	=	cpms.DP_CPM_A;//5.128205;
	DP_CPM_C	=	cpms.DP_CPM_C;//2.28205;
	DP_CPM_D	=	cpms.DP_CPM_D;//-0.139887;
	DP_CPM_B	=	cpms.DP_CPM_B;//-191.14102;

	LS_P_CPM	=	P_CPM_A*P_CPM+P_CPM_B;//工况压力修正公式
	printf("LS_P_CPM	=	P_CPM_A*P_CPM+P_CPM_B\n");
	printf("LS_P_CPM = %f P_CPM_A = %f P_CPM = %f P_CPM_B = %f\n",LS_P_CPM, P_CPM_A, P_CPM, P_CPM_B);
	G31_CPM		=	G31_CPM_A*LS_P_CPM+G31_CPM_B;
	printf("G31_CPM = G31_CPM_A*LS_P_CPM+G31_CPM_B\n");
	printf("G31_CPM = %f G31_CPM_A = %f LS_P_CPM = %f G31_CPM_B = %f\n",G31_CPM, G31_CPM_A, LS_P_CPM, G31_CPM_B);
	G356_CPM	=	G356_CPM_A*LS_P_CPM+G356_CPM_B;
	printf("G356_CPM	=	G356_CPM_A*LS_P_CPM+G356_CPM_B\n");
	printf("G356_CPM = %f G356_CPM_A = %f LS_P_CPM = %f G356_CPM_B = %f\n",G356_CPM, G356_CPM_A, LS_P_CPM, G356_CPM_B);
	shared->DataP	=	LS_P_CPM;
	TF_PCPM		=	T_P_CPM+273.15;//工况华式温度

	WD_CPM		=	cpms.WD_CPM;
	GD_CPM		=	GD_CPM_SC*TF_PCPM_SC*LS_P_CPM/(P_CPM_SC*TF_PCPM);//工况气密度
	printf("GD_CPM		=	GD_CPM_SC*TF_PCPM_SC*LS_P_CPM/(P_CPM_SC*TF_PCPM)\n");
	printf("GD_CPM = %f GD_CPM_SC = %f TF_PCPM_SC = %f LS_P_CPM = %f P_CPM_SC= %f TF_PCPM = %f\n",GD_CPM, GD_CPM_SC, TF_PCPM_SC, LS_P_CPM, P_CPM_SC, TF_PCPM);

	OVF_CPM		=	(((log((float)K31_CPM_Empty / (float)k31_CPM) / log(exp(1))) - D_CPM * G31_CPM) / (W31_CPM - G31_CPM)) / D_CPM;

	printf("	OVF_CPM		=	(((log(K31_CPM_Empty / k31_CPM) / log(exp(1))) - D_CPM * G31_CPM) / (W31_CPM - G31_CPM)) / D_CPM \n");
	printf("OVF_CPM = %f K31_CPM_Empty = %d k31_CPM = %d D_CPM = %f G31_CPM= %f W31_CPM = %f\n",OVF_CPM, K31_CPM_Empty, k31_CPM, D_CPM, G31_CPM, W31_CPM);
	if(OVF_CPM >= 0.0 && OVF_CPM <= 1.0){
		GVF_CPM		=	1-OVF_CPM;
	}
	else if(OVF_CPM < 0.0){
		OVF_CPM	=	0;
		GVF_CPM	=	1;
	}
	else if(OVF_CPM > 1.0){
		OVF_CPM	=	1.0;
		GVF_CPM	=	0.0;
	}


	OMF_CPM		=	OVF_CPM*WD_CPM/(OVF_CPM*WD_CPM+GVF_CPM*GD_CPM);
	printf("	OMF_CPM		=	OVF_CPM*WD_CPM/(OVF_CPM*WD_CPM+GVF_CPM*GD_CPM) \n");
	printf("OMF_CPM = %f OVF_CPM = %f WD_CPM = %f GVF_CPM = %f GD_CPM= %f \n",OMF_CPM, OVF_CPM, WD_CPM, GVF_CPM, GD_CPM);
	OMF_CPM		=	OMF_CPM_A*OMF_CPM+OMF_CPM_B;
	printf("		OMF_CPM		=	OMF_CPM_A*OMF_CPM+OMF_CPM_B; \n");
	printf("OMF_CPM = %f OMF_CPM_A = %f OMF_CPM_B = %f \n",OMF_CPM, OMF_CPM_A, OMF_CPM_B);

	if(OMF_CPM >= 0.0 && OMF_CPM <= 1.0){
		GMF_CPM		=	1.0-OMF_CPM;
	}
	else if(OMF_CPM < 0.0){
		OMF_CPM	=	0.0;
		GMF_CPM	=	1.0;
	}
	else if(OMF_CPM>1){
		OMF_CPM	=	1.0;
		GMF_CPM	=	0.0;
	}

	/*
	 * 相分计算完毕，后面计算总流量
	 */
	b_CPM			=	cpms.b_CPM;//0.5;
	C_CPM			=	cpms.C_CPM;//0.995;
	E_CPM			=	cpms.E_CPM;//1;
	LS_DP_CPM		=	DP_CPM_A*DP_CPM	+	DP_CPM_C*T_DP_CPM	+	DP_CPM_D*P_CPM	+	DP_CPM_B;//pa
	printf("LS_DP_CPM		=	DP_CPM_A*DP_CPM	+	DP_CPM_C*T_DP_CPM	+	DP_CPM_D*P_CPM	+	DP_CPM_B \n");
	printf("LS_DP_CPM = %f DP_CPM_A = %f DP_CPM = %f DP_CPM_C=%f T_DP_CPM=%f\n",LS_DP_CPM, DP_CPM_A, DP_CPM,DP_CPM_C,T_DP_CPM);
	printf("DP_CPM_D = %f P_CPM = %f DP_CPM_B = %f\n",DP_CPM_D, P_CPM, DP_CPM_B);

	printf("LS_DP_CPM =%f corrs.DP_th =%f\n",LS_DP_CPM, corrs.DP_th);
	if(LS_DP_CPM < corrs.DP_th){
		LS_DP_CPM = 0;
	}
	shared->DataDP	=	LS_DP_CPM;
	Pmix_CPM		=	WD_CPM*OVF_CPM	+	GD_CPM*GVF_CPM;
	printf("Pmix_CPM		=	WD_CPM*OVF_CPM	+	GD_CPM*GVF_CPM \n");
	printf("Pmix_CPM = %f WD_CPM = %f OVF_CPM = %f GD_CPM=%f GVF_CPM=%f\n",Pmix_CPM, WD_CPM, OVF_CPM,GD_CPM,GVF_CPM);
	x1				=	1-pow(b_CPM,4);
	x2				=	LS_DP_CPM*Pmix_CPM*2;
	if(x1<0||x1==0)
	{
		x1=1;
		printf("error\n");
	}
	if(x2<0)
	{
		x2=0;
	}

	printf("LS_DP_CPM =%f\n",LS_DP_CPM);
	Qv_CPM			=	0.004 * C_CPM * E_CPM * D_CPM * D_CPM * pow((LS_DP_CPM / Pmix_CPM), 0.5);
	printf("0.004 * C_CPM * E_CPM * D_CPM * D_CPM * pow((LS_DP_CPM / Pmix_CPM), 0.5)");
	printf("C_CPM = %f E_CPM = %f D_CPM = %f Pmix_CPM = %f",C_CPM,E_CPM,D_CPM,Pmix_CPM);

	Qm_CPM			=	Qv_CPM*Pmix_CPM;
	Qm_CPM			=	Qm_CPM_A*Qm_CPM	+	Qm_CPM_B;
	Qv_CPM			=	Qv_CPM_A*Qv_CPM	+	Qv_CPM_B;

	cpms.OVF_CPM	=	OVF_CPM;
	cpms.GVF_CPM	=	GVF_CPM;
	cpms.Qv_CPM		=	Qv_CPM;
	cpms.OMF_CPM	=	OMF_CPM;
	cpms.GMF_CPM	=	GMF_CPM;
	cpms.Qm_CPM		=	Qm_CPM;

	shared->OMF_CPM =	OMF_CPM;
	shared->GMF_CPM	=	GMF_CPM;
	shared->Qm_CPM	=	Qm_CPM;

	shared->Qm_SUM	=	Qm_CPM	+	shared->Qm_SUM;
    shared->Qml_CPM	=	Qm_CPM*OMF_CPM  + 	shared->Qml_CPM;
    shared->Qmg_CPM	=	Qm_CPM*GMF_CPM  + 	shared->Qmg_CPM;
    printf("shared->Qml_CPM	 = %f shared->Qmg_CPM = %f\n",shared->Qml_CPM,shared->Qmg_CPM);

}
