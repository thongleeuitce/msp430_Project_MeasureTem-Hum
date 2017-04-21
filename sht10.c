/*
 * sht10.c
 *
 *  Created on: Apr 19, 2017
 *      Author: ThongLee
 */
//define variables
#include "library/sht10.h"

void SHT10_init()
{
	P1SEL &= ~(SCK+SDA); 		//I/O function is selected
	P1DIR |= SCK;				//Output selected
	P1DIR &= ~SDA;				//Data is input
}

void SHT10_transmission_start()
{
	P1DIR |= SDA;			//Chan P1.4(DATA) duoc chon lam chan output
	SDA_H;					//P1OUT |= SDA
	SCK_L;					//P1OUT &= ~SDA
	SCK_H;
	SDA_L;
	SCK_L;
	SCK_H;
	SDA_H;
	SCK_L;
	P1DIR &= ~SDA;
}
unsigned int SHT10_WriteByte(unsigned char value)
{
	// writes a byte on the Sensibus and checks the acknowledge
	unsigned char i;
	unsigned int error = 0;
	P1DIR |= SDA; 	//Chan P1.4(DATA) duoc chon lam chan output
	for(i = 0x80; i > 0; i /= 2)            //shift bit for masking
	{
		if(i & value)
			SDA_H;            //masking value with i , write to SENSI-BUS
		else
			SDA_L;
		SCK_H;            //clk for SENSI-BUS
		_delay_cycles(5);		//pulswith approx. 5 us
		SCK_L;
	}
	SDA_H;                   //release DATA-line
	P1DIR &= ~SDA;           //Change SDA to be input 0:input 1:ouput
	SCK_H;                   //clk #9 for ack
	error = P1IN;           //check ack (DATA will be pulled down by SHT11)
	error &= SDA;
	P1DIR |= SDA;           //Change SDA to be output 0:input 1:ouput
	SCK_L;
	if(error)
		return 1;                     //error=1 in case of no acknowledge
	return 0;
}

unsigned char SHT10_ReadByte(unsigned char ack)
{
	// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1"
	unsigned char i, val = 0;
	P1DIR |= SDA;                      //Change SDA to be output 0:input 1:ouput
	SDA_H;                           //release DATA-line
	P1DIR &= ~SDA;                     //Change SDA to be input 0:input 1:ouput
	for(i = 0x80; i > 0; i /= 2)             //shift bit for masking
	{
		SCK_H;                     //clk for SENSI-BUS
		if(P1IN & SDA)
		  val = (val | i);               //read bit
		  SCK_L;
	}
	P1DIR |= SDA;                      //Change SDA to be output 0:input 1:ouput
	if (ack)                        //in case of "ack==1" pull down DATA-Line
		SDA_L;
	else
		SDA_H;
	SCK_H;                           //clk #9 for ack
	_delay_cycles(5);		///pulswith approx. 5 us
	SCK_L;
	SDA_H;                           //release DATA-line
	P1DIR &= ~SDA;                     //Change SDA to be output 0:input 1:ouput
	return val;
}


void SHT10_Connectionreset()
{
	unsigned char i;
	P1DIR |= SDA;                        //Change SDA to be input 0:input 1:ouput
	SDA_H;
	SCK_L;                         //Initial state
	for(i = 0; i < 9; i++)      //9 SCK cycles
	{
		SCK_H;
		SCK_L;
	}
	SHT10_transmission_start();                      //transmission start
}

unsigned int SHT10_Measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
	unsigned int error = 0;
	unsigned int i;

	SHT10_transmission_start();                   //transmission start
	switch (mode)
	{                                 //send command to sensor
		case TEMPERATURE:
			error += SHT10_WriteByte(MEASURE_TEMP);
		break;
		case HUMIDITY:
			error += SHT10_WriteByte(MEASURE_HUMI);
		break;
	}
	P1DIR &= ~SDA;             		//Change SDA to be input 0:input 1:ouput
	for (i = 0; i < 65535; i++)
		if((P1IN & SDA)==0)
			break; 					//wait until sensor has finished the measurement
	if(P1IN & SDA)
		error += 1;            		//or timeout (~2 sec.) is reached
	*(p_value) = SHT10_ReadByte(ACK);       //read the first byte (MSB) 8bit cao
	*(p_value + 1) = SHT10_ReadByte(ACK);   //read the second byte (LSB) 8bit thap
	*p_checksum = SHT10_ReadByte(NOACK);    //read checksum
	return error;
}

float SHT10_Calculate(float *f_temperature, float *f_humidity)
{
    //12 bit Humidity and 14 bit temperature
    // Humidity
    const float C1 = -2.0468;        //for 12 bit Humidity
    const float C2 = 0.0367;         //for 12 bit Humidity
    const float C3 = -0.0000015955;  //for 12 bit Humidity
    const float T1 = 0.01;           //for 12 bit Humidity
    const float T2 = 0.00008;        //for 12 bit Humidity
    // Temperature
    const float D1 = -39.7;              //for 3.5V
    const float D2 = 0.01;               //for 14 bit Temperature

	float SORH = *f_humidity;		//Humidity 12 Bit
	float SOT  = *f_temperature;		//Temperature 14 Bit
	float rh_Linear;                     	//Humidity linear
	float rh_True;                  			//Temperature compensation of Humidity Signal
	float temperature_C;                        // temperature_C : Temperature [℃]

	*f_temperature = D1 + D2*(*f_temperature);                //calc. temperature from ticks to [℃]
	*f_humidity = (*f_temperature - 25)*(T1 + T2*(*f_humidity)) + (C1 + C2*(*f_humidity) + C3*(*f_humidity)*(*f_humidity));   //calc. temperature compensated humidity [%RH]
	if(*f_humidity > 100)
	    *f_humidity = 100;         //cut if the value is outside of range
	if(*f_humidity < 0.1)
	    *f_humidity = 0.1;         //the physical possible range
}



