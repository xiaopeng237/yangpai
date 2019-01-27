/*
 * i2cwork.h
 *
 *  Created on: 2016-4-12
 *      Author: vmuser
 */
#ifndef I2CWORK_H
#define I2CWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>


#define I2C_ADDR  0xc0  //从机地址
#define I2C_ADDRV  0xc0  //从机地址
#define DATA_LEN 3
#define I2C_SLAVE	0x0703
#define I2C_TENBIT	0x0704
#define I2C_SLAVE_FORCE	0x0706	/* Use this slave address, even if it is already in use by a driver! */

#define I2C_1115_A	0x90
#define I2C_1115_B 0x92
#define I2C_1115_C 0x94
#define I2C_MUX_1	0x84
#define I2C_MUX_2	0xb4
#define I2C_RATE	0xe3

int i2c_CV(int addr);
int i2c_function(void);
int i2c_work(void);

int i2c_readADC_set(void);
int i2c_readADC_data(void);



#endif



