/*
 * main.c
 *
 *  Created on: Apr 19, 2017
 *      Author: NamNguyen
 */

#include "library/SHT10.h"
#include "library/UART.h"

float f_humi, f_temp;
    unsigned int uint_humi, uint_temp, error = 0;
    unsigned char checksum;
    unsigned int temphigh, templow, humihigh, humilow;


void main()
{
	WDTCTL = WDTPW+WDTHOLD; //Stop watchdog timer
	SHT10_init();               // initial SHT10
	UART_init();                // initial UART

	BCSCTL3 |= LFXT1S_2;
	TA0CCR0 = 15000;
	TA0CTL = TASSEL_1 + ID_3 + MC_1;

    SHT10_Connectionreset();

	TA0CCTL0 = CCIE;
	_BIS_SR(LPM3_bits|GIE);


}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void myTimer0ISR(void)
    {
        error = 0;
                error += SHT10_Measure((unsigned char*) &uint_humi, &checksum, HUMIDITY);   //measure humidity
                error += SHT10_Measure((unsigned char*) &uint_temp, &checksum, TEMPERATURE); //measure temperature
                if(error != 0)
                  SHT10_Connectionreset();          //in case of an error: connection reset
                else
                {
                  humihigh = ((uint_humi & 0x0f) << 8);
                  humilow = ((uint_humi & 0xff00) >> 8);
                  uint_humi = humihigh + humilow;       //Humidity
                  temphigh = ((uint_temp & 0x3f) << 8);
                  templow = ((uint_temp & 0xff00) >> 8);
                  uint_temp = temphigh + templow;       //Temperature

                  f_temp = (float)uint_temp;
                  f_humi = (float)uint_humi;
                  SHT10_Calculate(&f_temp, &f_humi);         //calculate humidity, temperature

                  UART_printf_string("Temperature: ");
                  UART_printf_float(f_temp, 2);
                  UART_printf_string("oC, ");
                  UART_printf_string("Humidity: ");
                  UART_printf_float(f_humi, 2);
                  UART_printf_string("%.\r\n");
                }
        //      UART_printf_string("Temperature: 12oC, ");
        //      UART_printf_string("Humidity: 34%.\r\n");
//              __delay_cycles(1000000*2);              // đelay 2s to avoid heating up SHT
    }

