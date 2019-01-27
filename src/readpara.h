/*
 * readpara.h
 *
 *  Created on: 2016-4-8
 *      Author: vmuser
 */
#ifndef READPARA_H
#define READPARA_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<linux/rtc.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<fcntl.h>
#include <sys/ioctl.h>
#include<unistd.h>
#include<errno.h>
#include<time.h>

int readpara(void);
int readtime(void);
int rtcinit(void);
int read_fpga_setpara(void);
int read_arm_para(void);
int read_flow_para(void);
int read_ad1115(void);

int write_5ms_data(void);
int write_logfile(void);
int pulse_data_w(void);
int spectral_data_w(void);
int write_es_data(void);
int flow_data_w(void);
int flow1_data_w(void);
int flow2_data_w(void);
int flow3_data_w(void);
int flow4_data_w(void);
int flow5_data_w(void);
int flow_table_w(void);
int arm_P(void);
void W_logfile(void);
void W_FlowData(void);
int write_mysql_data(void);

#endif



