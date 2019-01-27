/*
 * spi.h
 *
 *  Created on: 2016-4-8
 *      Author: vmuser
 */
#ifndef SPI_H
#define SPI_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>//sleep
#include <stdint.h>//uint
#include <fcntl.h>//ioctl
#include <sys/ioctl.h>
#include <math.h>//log
#include <linux/spi/spidev.h>
//#include <poll.h>
//#include <unistd.h>
#define WRITE  0
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

typedef unsigned char UCHAR;
typedef unsigned int  UINT;

static const uint8_t pulse_TX[2048]={0x0f,0x00,0x0f,0x00,0x0f,0x00,0x0f,0x00,0x0f,0x00
};

int spi_work(void);
int spi(void);
void setDefaultTh(void);
void SetTH(char num,unsigned int Value);
void SetTHS(char num,unsigned int Value);
void spiW(int tx_len, int rx_len ,unsigned char stx1, unsigned char stx2);
void spiR(int tx_len, int rx_len ,unsigned char stx1, unsigned char stx2);
int SPI_Transfer(const uint8_t *TxBuf, uint8_t *RxBuf, int len);

int pulse_rd(void);
int spectral_rd(void);
int spi_pulse(void);

int es_adjust1(void);

void adjust_win(void);

void errorCode(void);

#endif
