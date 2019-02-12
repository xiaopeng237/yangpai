/*
 * i2cwork.c
 *
 *  Created on: 2016-4-12
 *      Author: vmuser
 */
#include"i2cwork.h"
#include "data.h"
extern i2cdata i2c;
int GiFd;
unsigned int uiRet;

int i2c_function()
{
	GiFd = open("/dev/i2c-2",O_RDWR);
	if(GiFd == -1)
    	perror("open i2c-2\n");
	return GiFd;
}
int i2c_CV (int addr)
{
	int i;
	unsigned char tx_buf[DATA_LEN];
	uiRet = ioctl(GiFd, I2C_SLAVE, I2C_ADDR >> 1);
	if (uiRet < 0) {
		printf("setenv address faile ret: %x \n", uiRet);
		return -1;
	 }

	for (i = 0; i < DATA_LEN; i++) //写将要发送的数据到 发送buf
	    tx_buf[i] = 0x0a;

	tx_buf[0]=0x07;
	tx_buf[1]=0xff;
	write(GiFd, tx_buf, 2); //写数据
	return 0;
}

int i2c_Vadjust(float ratio,hander_result * p_result)
{
	if(p_result->addr >=p_result->high_v || p_result->addr <= p_result->low_v)
		p_result->addr = p_result->addrs;
//	printf("%.4xLLLLLLL",addr);
	if(ratio < 0.45)
		p_result->addr = p_result->addr + 5 ;
	else if(ratio >= 0.45 && ratio <= 0.48)
		p_result->addr = p_result->addr + 1;
	else if(ratio >0.52 && ratio <= 0.55)
		p_result->addr = p_result->addr -1;
	else if(ratio >= 0.55)
		p_result->addr = p_result->addr -5;
	else
		{
			p_result->peak_flag = 0;
		}
	i2c_CV(p_result->addr);

	return 0;
}
int i2c_readADC_set(void)
{
	unsigned char tx_adcbuf[DATA_LEN];
	uiRet = ioctl(GiFd, I2C_SLAVE_FORCE, I2C_1115_A >> 1);
	if (uiRet < 0) {
		printf("setenv address faile ret: %x \n", uiRet);
		return -1;
	}
	tx_adcbuf[0]=0x01;
	tx_adcbuf[1]=I2C_MUX_1;
	tx_adcbuf[2]=I2C_RATE;
	write(GiFd, tx_adcbuf, 3); //写数据
	tx_adcbuf[0]=0x00;
	write(GiFd,tx_adcbuf,1);
		return 0;
}
int i2c_readADC_data(void)
{
	unsigned char rx_adcbuf[2];
//	usleep(5000);
	read(GiFd, rx_adcbuf, 2);
//		printf("%2d,%2d\n",rx_buf[0],rx_buf[1]);
	i2c.ad1 = ((float)rx_adcbuf[0]*256+rx_adcbuf[1])*2.048/32767.0;
	printf("%f\n",i2c.ad1);
	return 0;
}

int i2c_work(void)
{
	//读取adc数据
	i2c_readADC_data();
	return 0;
}




