/*
 * dataacq.h
 *
 *  Created on: 2016-4-8
 *      Author: vmuser
 */

#ifndef DATAACQ_H
#define DATAACQ_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <signal.h>//记时函数头文件
#include <sys/time.h>

#include <time.h>//#include <stdio.h>

int dataacq(void);
int interrupt(void);
int intersignal(void);




#endif

