/*
 * Fcalculate.h
 *
 *  Created on: 2016-6-13
 *      Author: vmuser
 */

#ifndef __FCALCULATE_H__
#define __FCALCULATE_H__

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
	} ResultPVT;
int fcalculate(void);
int texts(void);
void PVTcalculations(void);//计算PVT
void PhaseFractionCalculates(void);//计算相分率PFC
void normalPVT(void);
void blackOilModels(void);
void ThirdEnergyPFC(void);
void LowCountThirdEnergyPFC(void);
void LowCountDoubleEnergyPFC(void);
void CPM(void);

#endif
