/*
 * spmath.c
 *
 *  Created on: 2016-4-11
 *      Author: vmuser
 */
#include "spmath.h"
#include "flow_calculation.h"
#include "data.h"
#include <math.h>
#include <stdio.h>

extern nMVT 		s_nMVT;
extern modelParameter		m_modelParameter;
extern resultPVT 		s_resultPVT;
extern resultPVT       m_resultPVT;
float        paraPVTSim[90];
extern publicParameter 	m_publicParameter;

measureResult s_measureResult; //seconds值
extern nVenturi			m_nVenturi;

extern calParameter highCalParameter;
extern calParameter lowCalParameter;
extern dualGamma 	s_dualGamma;

#define K   273.15
#define TEST    1
#define max(a, b)  ((a>b) ? a:b)
#define min(a, b)  ((a>b) ? b:a)

void test(void)
{
	s_nMVT.temperature =21.7834;
	s_nMVT.pressure = 565.548828;
	s_nMVT.diffPressure =189.575668;
	s_dualGamma.highEnergyCount     =300000/60;
	s_dualGamma.lowEnergyCount 		=300000/60;
}

void WLRGVFCalculate(float Le,float He)
{
	float oilDensityLC =s_measureResult.oilDensityLC;
	float waterDensityLC =s_measureResult.waterDensityLC;
	float gasDensityLC =s_measureResult.gasDensityLC;

	float uWH = highCalParameter.waterAbsorptionCoefficient*waterDensityLC/10;//质量吸收系数变为线性吸收系数 质量吸收系数拿来的？？？
	float uOH = highCalParameter.oilAbsorptionCoefficient*oilDensityLC/10;//吸收系数单位cm2/g转换为m3/kg
	float uGH = highCalParameter.gasAbsorptionCoefficient*gasDensityLC/10;//吸收系数单位cm2/g转换为m3/kg

	float uWL = lowCalParameter.waterAbsorptionCoefficient*waterDensityLC/10;
	float uOL = lowCalParameter.oilAbsorptionCoefficient*oilDensityLC/10;
	float uGL = lowCalParameter.gasAbsorptionCoefficient*gasDensityLC/10;
//printf(",%f-%f-%f,",uWH,uOH,uGH);
	float H = He;//log(NEH/NMH)/aD;//高能线性衰减系数 打算在别处算好
	float L = Le;// log(NEL/NML)/aD;


	float uMH = uWH-uOH;
	float uML = uWL-uOL;

	float Rg = ((uML*H-uMH*L)-(uOH*uML-uOL*uMH))/((uML*uGH-uMH*uGL)-(uOH*uML-uOL*uMH));//GVF2 体积含气率
//	if((Rg<0)||(Rg>1))	{Rg = 0;}
	float Rw =  (L-uOL*(1-Rg)-uGL*Rg)/(uML*(1-Rg));//持夜率 LHU   ~~~~~~~~~~~~~~~~~~~~~~~~~
//	if((Rw<0)||(Rw>1))	{Rw = 0;}
	float LHU =  (L-uOL*(1-Rg)-uGL*Rg)/uML;//持夜率 LHU

	float C3 = m_modelParameter.nC3;//nc3是补偿系数
	s_measureResult.dualGammaLHU =LHU;
	s_measureResult.dualGammaGVF =Rg;
	s_measureResult.dualGammaWLR =Rw+C3; //含水率为啥加nc3？？？？？？？？？？？？？
	s_measureResult.flowMixDensity = (gasDensityLC *Rg + waterDensityLC*(1-Rg)*(Rw+C3) +  oilDensityLC *(1-Rg)*(1-Rw-C3));//dqz
//	printf(",GVF is %f,LHU is %f WLR is %f  ,mixDen is %f,\n",Rg,LHU,Rw,s_measureResult.flowMixDensity);//混合密度
}

float normalOil(void)
{
  float visO =0;
  float deltaO = m_publicParameter.oilDensitySC/1000;
  float  t     = s_nMVT.temperature;
  float z = 5.6926-2.8625/deltaO;
  float y = pow(10,z);
  float x0 = 1.8*t+32;
  float x = y* pow(x0,-1.163);
  visO = pow(10,x)-1;
//  printf("$$%f$$",visO);
  return visO;


}
float heavyOilCurve(void)
{
	float a = m_modelParameter.oilVisA;
	float b = m_modelParameter.oilVisB;
	float c = m_modelParameter.oilVisC;
	float  t     = s_nMVT.temperature;
	float visO = a* pow(t,2)+b*t+c;
	return visO;
}

float heavyOilThreoy(void)
{
	float vis50 = m_modelParameter.Reserve1;
	float  t     = s_nMVT.temperature;
	float v0 = 3.1613 - 0.5525*log(t);
	float visO = (0.0148*log(t)+0.9421)*pow(vis50,v0);
	return visO;
}
/*********************************************************************************************************
** Function name:      void viscosityCalculate(void )
** Descriptions:      粘度计算
** input parameters:    无
** output parameters:   无
** Returned value:
*********************************************************************************************************/
void viscosityCalculate(void )
{
   float visO =0;
   int oilVisSel =2;//(int) m_modelParameter.oilVisSel;
   switch(oilVisSel)
   {
   	case 1: visO =heavyOilCurve();
	        break;
    case 2:
			visO =normalOil();
			break;
	case 3: visO =heavyOilThreoy();
	        break;
	default :
	        break;
   }

   float Rs = s_resultPVT.Rs ;
   float A0 = 0.00455*Rs;
   float A  =0.2+ 0.8/pow(10,A0);
   float B0 = 0.004045*Rs;
   float B  = 0.43 +0.57/pow(10,B0);
   float visONg = A*pow(visO,B);
   float WLR = s_measureResult.dualGammaWLR;
   float  t     = s_nMVT.temperature;
   float  betaO = m_publicParameter.oilExpansionFactor;
   float deltaO = m_publicParameter.oilDensitySC;
   float oilDen1556 = deltaO /(1-betaO*(t-15.56));
   		//oilDen1556 = m_publicParameter.oilDensitySC;//dqz 采用标况下密度，而不是15.56度下的密度
   float Mo = 1.806*oilDen1556-1246;


   float oilDensityLC = s_measureResult.oilDensityLC;
   float waterDensityLC = s_measureResult.waterDensityLC;
   float gasDensityLC=s_measureResult.gasDensityLC;

   float Xw = WLR*waterDensityLC*Mo/(WLR*waterDensityLC*Mo+18*(1-WLR)*oilDensityLC);//???????????
   float Xo = 1-Xw;
   float visW;
	if(t<20)
		visW = 1.002;
	else if(t>80)
		visW = 0.355;
	else
		visW=0.000145*t*t-0.025283*t+1.4497;
   float visOLC0  = pow(visONg,1.0/3)*(1-WLR)+pow(visW,1.0/3)*WLR;
   float visOLC   = pow(visOLC0,3);

   float beta = s_measureResult.dualGammaGVF;
   float visG = 0.012;
   float X = beta *gasDensityLC/((1-WLR)*(1-beta)*oilDensityLC+WLR*(1-beta)*waterDensityLC+beta*gasDensityLC);
   float visMix=0;
   if(m_modelParameter.mixVisSel==1)
   {
		visMix = X *visG+(1-X)*visOLC;
   }
   else if(m_modelParameter.mixVisSel==2)
   {
		visMix =beta*visG+(1-beta)*visOLC;
   }
   else
   {
   }
   s_measureResult.mixViscosity = visMix;
 //  printf("visMix is %f \n",visMix);
}

float getReFactor( float C)
{
    float E = m_nVenturi.venturi_E;
	float d = m_nVenturi.venturi_d;
	float D = m_nVenturi.venturi_D;
	float dp = s_nMVT.diffPressure;  // Pa
	float DenM = s_measureResult.flowMixDensity;
	float Vis = s_measureResult.mixViscosity;
	float ZERO_VAL=0.00001;
	float Qv = 0.004*C*E*d*d*sqrt(dp*1000/DenM);
	float Qm = Qv*DenM;
	float Re;
	if(D*Vis<ZERO_VAL)
		Re = 300000;
	else
		Re = (354*Qm/(D*Vis));
	if(Re<ZERO_VAL)
		Re = 300000;
	return Re;
}

float getCFactor(float Re)

{
	float C;
	if(Re<=2000)
		C = 0.0785*log(Re)+0.2945;
	else if(Re>=2000 && Re<=100000)//dqz
		C = 0.017*log(Re)+0.7859;
		else
		C = 0.995;
		return C;

}

float  interationC(void)
{
    int i;
	float c1 =0.995;
	float c2 =0 ;
    float	Re = getReFactor(c1);
	for(i=0;i<1000;i++)
	{
		if(fabs(c1-c2)>=0.001)
		{
			c2 =c1;
			c1 = getCFactor(Re);
			Re = getReFactor(c1);
		}

	    else
	    {
	      return c1;
	    }
	}
	return c1;
}

void selCFactor(void)
{
	float  C =0.995;
	if( m_modelParameter.Reserve4 ==0)
	{
	   C = interationC();
	}
	else if( m_modelParameter.Reserve4 ==1)
	{
		C = m_nVenturi.venturi_C;
	}
	else
	{
	}
		s_measureResult.nDischargeCoefficient =C;
		s_measureResult.Red = getReFactor(C);
	//	printf("Reserve4 is %f Red is %f C is %f\n",m_modelParameter.Reserve4,s_measureResult.Red,C );
}

/*void normalPVT(void)
{
    float  betaO = m_publicParameter.oilExpansionFactor;//工况标况密度比 工况=标况/b0？？？？膨胀洗漱怎么得来的？？？
	float  betaW = m_publicParameter.waterExpansionFactor;
//	printf(",!!%f,",m_publicParameter.oilExpansionFactor);
	float  P0    = m_publicParameter.localBarometricPressure;//工况气压adc赋值
	float  P     = s_nMVT.pressure;//标况气压
	float  t     = s_nMVT.temperature;//标况温度
	float  t0    = m_publicParameter.referenceTemperatureSC;//工况温度adc赋值
	float  t_K   = t + K;//摄氏度变华氏度
	float  t0_K  = t0+ K;
//    float  temperatureK = s_nMVT.temperature +K;
	float  oilDensitySC = m_publicParameter.oilDensitySC;//标况密度哪里得到？？？？？？
	float  waterDensitySC = m_publicParameter.waterDensitySC;
	float  gasDensitySC = m_publicParameter.gasDensitySC;
	float  oilDensityLC,waterDensityLC,gasDensityLC;//工况密度
    s_resultPVT.Bo	= 	1/(1-betaO *(t-t0));//油气水三者的工况标况密度比
	s_resultPVT.Bw 	=	1/(1-betaW *(t-t0));
	s_resultPVT.Bg	= 	(P0*t_K)/(P*t0_K);
	s_resultPVT.Z 	= 	1;//气体压缩因子
	s_resultPVT.Rs 	=	0;//溶解气  为什么是0？

	oilDensityLC =  oilDensitySC/s_resultPVT.Bo ;//转化为工况密度
    waterDensityLC = waterDensitySC/s_resultPVT.Bw;
	gasDensityLC  = gasDensitySC/s_resultPVT.Bg;

	s_measureResult.oilDensityLC = oilDensityLC;
	s_measureResult.waterDensityLC = waterDensityLC;
	s_measureResult.gasDensityLC = gasDensityLC;
//	printf("%f,%f,%f",s_measureResult.oilDensityLC,s_measureResult.waterDensityLC,s_measureResult.gasDensityLC);
}*/
void blackOilModel(void)
{
//	test();
	float deltaG 			=	m_publicParameter.Gr;
	float Gr 				= 	m_publicParameter.Gr;
	float Mc 				= 	m_modelParameter.nMC;
	float Mn 				= 	m_modelParameter.nMn;
	float Mc1 				= 	m_modelParameter.nMC1;
	float Zn  				=	m_modelParameter.nZn;
	float P0 				= 	m_publicParameter.localBarometricPressure/1000;
	float P   				= 	s_nMVT.pressure/1000;
	float t   				= 	s_nMVT.temperature;
	float betaO 			= 	m_publicParameter.oilExpansionFactor;
	float oilDensitySC 		= 	m_publicParameter.oilDensitySC;
	float waterDensitySC 	= 	m_publicParameter.waterDensitySC;
	float gasDensitySC 		= 	m_publicParameter.gasDensitySC;
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
	 }
	 else
	    E = 0;
	m=0.0330378*pow(tao,-2)-0.0221323*pow(tao,-3)+0.0161353*pow(tao,-5);
	n=(-0.133185*pow(tao,-1)+0.265827*pow(tao,-2)+0.0457697*pow(tao,-4))/m;
	B = (3-m*pow(n,2))/(9*m*pow(H,2));
    b = (9*n-2*m*pow(n,3))/(54*m*pow(H,3))-E/(2*m*pow(H,2));
	float D0 = pow(b,2)+pow(B,3);
	float D1 = sqrt(D0)+b;
	float D2 =1.0/3;
	D = pow(D1,D2);
    float Fz0 = B/D-D+n/(3*H);
	Fz = sqrt(Fz0)/(1+0.00132/pow(tao,3.25));
	Z =Zn/pow(Fz,2);


	float T0 = m_publicParameter.referenceTemperatureSC +K;
	float Pb = m_modelParameter.nPb/1000;
	float T  =t +K;
	float Bg = Z*P0*T/(P*T0);

    float deltaO = m_publicParameter.oilDensitySC/1000;
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

	s_resultPVT.Bo	= 	Bo;
	s_resultPVT.Bw 	=	Bw;
	s_resultPVT.Bg	= 	Bg;
	s_resultPVT.Z 	= 	Z;
	s_resultPVT.Rs 	=	Rs;

	oilDensityLC = (oilDensitySC+ Rs *deltaGs*gasDensitySC/Gr)/Bo;
	gasDensityLC  = gasDensitySC/Bg;
	waterDensityLC = waterDensitySC/Bw;
    //printf("result is %f %f %f %f %f %f %f %f\n",Bo,Bw,Bg,Z,Rs,oilDensityLC,waterDensityLC,gasDensityLC);
	s_measureResult.oilDensityLC = oilDensityLC;
	s_measureResult.waterDensityLC = waterDensityLC;
	s_measureResult.gasDensityLC = gasDensityLC;

}

void customModel(void)

{
	float  Bo,Bw,Bg,Z,Rs;
	float  oilDensityLC,waterDensityLC,gasDensityLC;
  	float P0 = m_publicParameter.localBarometricPressure/1000;
	float T0 = m_publicParameter.referenceTemperatureSC +K;
	float  oilDensitySC = m_publicParameter.oilDensitySC;
	float  waterDensitySC = m_publicParameter.waterDensitySC;
	float  gasDensitySC = m_publicParameter.gasDensitySC;
	float P   = s_nMVT.pressure/1000;
	float t   = s_nMVT.temperature;
	float T  =t +K;
	Z  = m_modelParameter.zFactor;
	Bg = Z*P0*T/(P*T0);
	Bo = 1/m_modelParameter.oilShrinkageFactor;
	Bw = 1.0088 -4.4748*P*pow(10,-4)+6.2666*P*pow(10,-7);
	Rs = m_modelParameter.nGor;

    s_resultPVT.Bo	= 	Bo;
	s_resultPVT.Bw 	=	Bw;
	s_resultPVT.Bg	= 	Bg;
	s_resultPVT.Z 	= 	Z;
	s_resultPVT.Rs 	=	Rs;

	oilDensityLC =  oilDensitySC/Bo ;
    waterDensityLC = waterDensitySC/Bw;
	gasDensityLC  = gasDensitySC/Bg;

	s_measureResult.oilDensityLC = oilDensityLC;
	s_measureResult.waterDensityLC = waterDensityLC;
	s_measureResult.gasDensityLC = gasDensityLC;
//	printf("result is3 %f %f %f %f %f %f %f %f\n",Bo,Bw,Bg,Z,Rs,oilDensityLC,waterDensityLC,gasDensityLC);
}
void PVTsimModel(void)
{
	float p,t;
	p = s_nMVT.pressure;
	t = s_nMVT.diffPressure;
	m_resultPVT.Bo = paraPVTSim[11]+paraPVTSim[12]*p+paraPVTSim[13]*t+paraPVTSim[14]*p*p/10000+paraPVTSim[15]*p*t/100+paraPVTSim[16]*t*t;
	m_resultPVT.Rs = paraPVTSim[21]+paraPVTSim[22]*p+paraPVTSim[23]*t+paraPVTSim[24]*p*p/10000+paraPVTSim[25]*p*t/100+paraPVTSim[26]*t*t;
	m_resultPVT.Bg = paraPVTSim[31]+paraPVTSim[32]*p+paraPVTSim[33]*t+paraPVTSim[34]*p*p/10000+paraPVTSim[35]*p*t/100+paraPVTSim[36]*t*t;
	m_resultPVT.Rv = paraPVTSim[41]+paraPVTSim[42]*p+paraPVTSim[43]*t+paraPVTSim[44]*p*p/10000+paraPVTSim[45]*p*t/100+paraPVTSim[46]*t*t;
	m_resultPVT.Bw = paraPVTSim[51]+paraPVTSim[52]*p+paraPVTSim[53]*t+paraPVTSim[54]*p*p/10000+paraPVTSim[55]*p*t/100+paraPVTSim[56]*t*t;
	m_resultPVT.oilDensityLC = paraPVTSim[61]+paraPVTSim[62]*p+paraPVTSim[63]*t+paraPVTSim[64]*p*p/10000+paraPVTSim[65]*p*t/100+paraPVTSim[66]*t*t;
	m_resultPVT.waterDensityLC = paraPVTSim[71]+paraPVTSim[72]*p+paraPVTSim[73]*t+paraPVTSim[74]*p*p/10000+paraPVTSim[75]*p*t/100+paraPVTSim[76]*t*t;
	m_resultPVT.gasDensityLC = paraPVTSim[81]+paraPVTSim[82]*p+paraPVTSim[83]*t+paraPVTSim[84]*p*p/10000+paraPVTSim[85]*p*t/100+paraPVTSim[86]*t*t;

}
void PVTcalculation(void)
{
	int PVTmodel = 1;
			//(int) m_modelParameter.nPVTModelSelect;

   switch(PVTmodel)
   {
    case 0 	:   normalPVT();
				break;
	case 1  :	blackOilModel();
				break;
	case 2  :	customModel();
	            break;
	case 3  :	PVTsimModel();
				break;
	default :   break;

   }
}


void flowcalculation(float L,float H)
{
	if(TEST)
	test();

	PVTcalculation();
	WLRGVFCalculate(L,H);
	viscosityCalculate();//粘度计算
	selCFactor();
	float E = m_nVenturi.venturi_E;
	float d = m_nVenturi.venturi_d;
	float D = m_nVenturi.venturi_D;
	float dp = 	 s_nMVT.diffPressure;  // Pa
	float DenM = s_measureResult.flowMixDensity;
	float Vis =  s_measureResult.mixViscosity;
	float C =    s_measureResult.nDischargeCoefficient;
	float WLR =  s_measureResult.dualGammaWLR;
	float GVF =  s_measureResult.dualGammaGVF;
	float Qv = 0.004*C*E*d*d*sqrt(dp*1000/DenM) *24; //m3/d
//	printf("#####%f%f%f%f#####\n",C,D,E,d);
	float QvGasLC = Qv*GVF;
	float QvLiqLC = Qv-QvGasLC;
	float QvWaterLC = QvLiqLC*WLR;
	float QvOilLC = QvLiqLC-QvWaterLC;

	///
	float C1 = m_modelParameter.nC1;
	float C2 = m_modelParameter.nC2;

	float sA  = m_modelParameter.slipA;
	float sB  = m_modelParameter.slipB;
	float sC  = m_modelParameter.slipC;
	float Bw = s_resultPVT.Bw;
	float Bo = s_resultPVT.Bo;
	float Bg = s_resultPVT.Bg;
	float Rs = s_resultPVT.Rs;

	float QvLiqLC1 = C1 * QvLiqLC;
	float QvGasLC1 =  QvGasLC;//C2 *
	float QvWaterLC1 = QvLiqLC1*WLR;

	float QvOilLC1   = QvLiqLC1 - QvWaterLC1;
	float slipRatio = 1+0.01*(sA*QvLiqLC1+sB*GVF+sC);
	float QvGasLC2 = QvGasLC1/slipRatio;

	float GVF1 = QvGasLC2/(QvGasLC2+QvLiqLC1);//体积含气率

	float QvWaterSC = QvWaterLC1 /Bw;
	float QvOilSC =   QvOilLC1 /Bo;
	float QvLiqSC =  QvWaterSC+QvOilSC;
	float QvGasSC = QvGasLC2/Bg +Rs*QvOilSC;
	float WLRSC = QvWaterSC/QvLiqSC;

	if(C2==1)//湿气修正选择
	{
		float FAI0 = 1;  	//文丘里初始修正因子
		float C0 =1;		//文丘里初始流出系数
		float oilDensityLC = s_measureResult.oilDensityLC;//kg/m3
		float waterDensityLC = s_measureResult.waterDensityLC;
		float gasDensityLC = s_measureResult.gasDensityLC ;

//		float MWetGas0 = QvGasLC2 * gasDensityLC;//（第0次）初始气体质量流量:kg/d
		float MWetGas0 = C0 * E * 3.1415926/4 * 0.9942*pow(d/1000, 2)* sqrt(2*gasDensityLC*dp*1000)/FAI0;

		float liquidDensityLC = ((1-WLR)* oilDensityLC + WLR * waterDensityLC);//液体密度 kg/m3
		float MLiquid = QvLiqLC1 * liquidDensityLC/60/60/24;//初始液体质量流量 kg/s
//		float MLiquid_d = QvLiqLC1 * liquidDensityLC;//初始液体质量流量 kg/d

		float m1 = MWetGas0;//	kg/s
		float m2 = 0;

		while(fabs(m1-m2)>=0.001)
		{
			m2 = m1;
			float xi = (MLiquid/m1) * sqrt(gasDensityLC/liquidDensityLC);
			float FriGas = (4*m2*sqrt(gasDensityLC/(liquidDensityLC-gasDensityLC)))/(gasDensityLC*3.1415926*pow(D/1000,2)*sqrt(9.81*D/1000));
			float ni = max(0.583-0.18*pow(d/D,2)-0.578*exp(-0.8*FriGas/1), 0.392-0.18*d/D);
			float ci_CH = pow(liquidDensityLC/gasDensityLC, ni) + pow(gasDensityLC/liquidDensityLC, ni);
			float faii = sqrt(1+ci_CH*xi+pow(xi,2));

			float FriGasTh = FriGas/pow(d/D,0.5);
			float Ci = 1-0.0463*exp(-0.05*FriGasTh)*min(1, sqrt(xi/0.016));

			m1 = Ci * E * 3.1415926/4 * 0.9942* pow(d/1000, 2) * sqrt(2*gasDensityLC*dp*1000)/faii;//单位kg/s
		}
		QvGasLC2 = m1*60*60*24/gasDensityLC;
		QvGasSC = QvGasLC2/Bg +Rs*QvOilSC;
	}

	s_measureResult.liquidFlowLC 	= QvLiqLC1;
	s_measureResult.oilFlowLC		= QvOilLC1;
	s_measureResult.waterFlowLC		= QvWaterLC1;
	s_measureResult.gasFlowLC		= QvGasLC2;

	s_measureResult.liquidFlowSC 	= QvLiqSC;
	s_measureResult.oilFlowSC   	= QvOilSC;
	s_measureResult.waterFlowSC		= QvWaterSC;
	s_measureResult.gasFlowSC       = QvGasSC;

	s_measureResult.waterLiquidRatio= WLRSC;
	s_measureResult.gasVoidFraction = GVF1;

	s_measureResult.nSlipRatio 		= slipRatio;

	s_measureResult.temperature = s_nMVT.temperature;
	if(s_nMVT.temperature>100)
	{
		printf("flowcalculation():temperature is too high!!\n");
	}
	s_measureResult.pressure = s_nMVT.pressure;
	s_measureResult.diffPressure = s_nMVT.diffPressure;

	s_measureResult.highEnergyCount = s_dualGamma.highEnergyCount;
	s_measureResult.lowEnergyCount = s_dualGamma.lowEnergyCount;
}





