/*
 * SHT10.h
 *
 *  Created on: Apr 19, 2017
 *      Author: ThongLee
 */

#include <msp430g2553.h>
#include <stdio.h>

#ifndef SHT10_H_
#define SHT10_H_

//datasheet SHT10 command define
#define STATUS_REG_W 0x06   // binary: 0000 0110
#define STATUS_REG_R 0x07   // binary: 0000 0111
#define MEASURE_TEMP 0x03   // binary: 0000 0011
#define MEASURE_HUMI 0x05   // binary: 0000 0101
#define RESET      0x1e   // binary: 0001 1110

// SHT10 mode define
#define SHT_12_8_BIT   	0x01
#define SHT_14_12_BIT   0x00

#define NOACK 0                // no acknowledge
#define ACK 1                   // acknowledge
#define TEMPERATURE 1           // Temperature mode
#define HUMIDITY 2              // humidity mode

// Initial state for SCK and SDA
#define SCK         	BIT5	//P1.5
#define SDA         	BIT4	//P1.4
#define SCK_H       	P1OUT |= SCK
#define SCK_L       	P1OUT &= ~SCK
#define SDA_H       	P1OUT |= SDA
#define SDA_L       	P1OUT &= ~SDA
#define SDA_OUT         P1DIR |= SDA
#define SDA_IN          P1DIR &= ~SDA

// Function SHT10
void SHT10_init();
void SHT10_transmission_start();
unsigned int SHT10_WriteByte(unsigned char value);
unsigned char SHT10_ReadByte(unsigned char ack);
void SHT10_Connectionreset();
unsigned int SHT10_Measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode);
float SHT10_Calculate(float *f_temperature, float *f_humidity);

#endif /* LIBRARY_SHT_H_ */
