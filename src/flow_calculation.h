/*
 * flow_calculation.h
 *
 *  Created on: 2016-4-11
 *      Author: vmuser
 */
#ifndef __HAIMOPROTOCOL_H__
#define __HAIMOPROTOCOL_H__

typedef struct {
	char engineerName[8];
	char customerName[8];
	char oilFiledName[8];
	char stationName[8];
	char deviceNumber[10];
	char softwareVersion[10];
}backgroundInfo;
///SC Standard condition
typedef struct {
	float  	oilExpansionFactor;
	float	waterExpansionFactor;
	float	oilDensitySC;
	float 	waterDensitySC;
	float 	gasDensitySC;
	float	localBarometricPressure;
	float   referenceTemperatureSC;
	float   Gr;                      // gas specific gravity
	}publicParameter;
typedef struct {
	float upLimitValue;
	float downLimitValue;
}nADChannel;
typedef  struct {
	float  	venturi_C;
	float 	venturi_E;
	float   venturi_d;
	float   venturi_H;
	float   venturi_D;
	}nVenturi;
typedef struct {
    float  temperature;
	float  pressure;
	float  diffPressure;
	}nMVT;
typedef struct {
    float highEnergyCount;
	float lowEnergyCount;
	}dualGamma;
typedef struct {
    float	oilShrinkageFactor;
	float	zFactor;
	float   nGor;
	float	diffPressureThreshod;
	float 	nPVTModelSelect;
	float	nC1;
	float	nC2;
	float	nC3;
	float	nC4;
	float	nC5;
	float	nC6;
	float	nZn;
	float	nMn;
	float	nMC;
	float	nMC1;
	float	nPb;
	float	mixVisSel;
	float	oilVisSel;
	float	oilMolecularWeight;
	float	oilVisA;
	float	oilVisB;
	float	oilVisC;
	float	slipA;
	float	slipB;
	float	slipC;
	float   Reserve1;
	float   Reserve2;
	float   Reserve3;
	float   Reserve4;
	float   Reserve5;
	float   Reserve6;
	float   Reserve7;
	float   Reserve8;
	float   Reserve9;
	float   Reserve10;
}modelParameter;

 typedef struct {
	unsigned char 	nBAS;
	unsigned char 	function;
	unsigned short 	dataNumber;
 }packageHead;

 typedef struct {
    float	emptyPipeCount;
	float	gasAbsorptionCoefficient;
	float	waterAbsorptionCoefficient;
	float	oilAbsorptionCoefficient;
	float	absorptionDistance;
	float	calTemperature;
	}calParameter;
typedef struct {
			float Bo;
			float Bw;
			float Bg;
			float Z;
			float Rs;

			float Rv;
			float oilDensityLC;
			float waterDensityLC;
			float gasDensityLC;
	} resultPVT;

 typedef struct {
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
	}measureResult;
	void HMPrInit(void);
	void functionProcess(void);
	void ProErrorRes( int type);
	void readCount(void);
	void readMVT(void);
	void writeLofile(void);
	void AccAvgResult(void);
	void getResult(float L,float H);
	void readLogfile(char *name);
	int loadConfig(void);
	void loadPara1(void);
	void loadPara2(void);
	void loadPara3(void);
	void loadPara4(void);
	void loadCali(void);
	void demarcate(void);
	void emptycon(void);
	void oilcon(void);
	void gascon(void);
	void watercon(void);

#endif



