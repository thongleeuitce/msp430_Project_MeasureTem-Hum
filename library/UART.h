/*
 * UART.h
 *
 *  Created on: Apr 19, 2017
 *      Author: ThongLee
 */

#include <msp430g2553.h>
#include <stdio.h>
#ifndef UART_H_
#define UART_H_

#define RX BIT1     // P1.1
#define TX BIT2     // P1.2

void UART_init(void);						       	//Khoi tao UART
void _config_clock(void);
void UART_printf_char(unsigned char m_char);		  	//Gui chuoi ki tu
void UART_printf_string(unsigned char* string);		   	//Goi so kieu int
void UART_printf_int(unsigned long number);			//In ra gia tri bit cua thanh ghi
void UART_printf_float(float m_float, unsigned int m_int);		//coma<=4

void _config_clock(void)
{
    if(CALBC1_1MHZ == 0xFF)
        while(1);
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    BCSCTL2 = SELM_0 | DIVM_0;
}

void UART_init(void)
{
	_config_clock();
	
	P1SEL = RX | TX;            // P1.1 is RX, P1.2 is TX
    P1SEL2 = RX | TX;
    UCA0CTL1 = UCSWRST | UCSSEL_2;

    UCA0CTL0 = 0x00;
    UCA0MCTL = UCBRF_8 | UCBRS_0 | UCOS16;
    UCA0BR0 = 6;
    UCA0BR1 = 00;

    UCA0CTL1 &= ~UCSWRST;
    IE2 = UCA0RXIE;
    _BIS_SR(GIE);
}
void UART_printf_int(unsigned long number)
{
    unsigned char buffer[16];
    unsigned int i, j;

    if(number < 0)
        UART_printf_char('-');
    if (number == 0)
    {
        UART_printf_char('0');
        return;
    }
    for(i = 15; i>=0 ; i--)
    {
        buffer[i] = number%10 +'0';
        number = number/10;
        if(number == 0)
            break;
    }
    for(j = i; j<16; j++)
        UART_printf_char(buffer[j]);
}
void UART_printf_char(unsigned char m_char)
{
    while(!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = m_char;
}
void UART_printf_string(unsigned char* string)
{
    unsigned int i;
    for (i = 0; i< strlen(string); i++)
        UART_printf_char(string[i]);
}

void UART_printf_float(float m_float, unsigned int m_int)
{
    unsigned long temp;
    float x;
    if (m_float < 0)
    {
        UART_printf_char('-');
        m_float *= -1;
    }
    temp = (unsigned long) m_float;
    UART_printf_int(temp);
    x = m_float - (float)temp;
    while(m_int > 0)
    {
        x *= 10;
        m_int --;
    }
    UART_printf_char('.');
    x = (unsigned long)(x + 0.5);   // Lam tron
    UART_printf_int(x);
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_IRS (void)
{
}

#endif	/* UART_H_ */
