/*
 * gpio.h
 *
 *  Created on: 2016-4-8
 *      Author: vmuser
 */
#ifndef GPIO_H
#define GPIO_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#define MSG(args...) printf(args)
int gpio_export(int pin);
int gpio_unexport(int pin);
int gpio_direction(int pin, int dir);
int gpio_write(int pin, int value);
int gpio_read(int pin);
int gpio_edge(int pin, int edge);

#endif
