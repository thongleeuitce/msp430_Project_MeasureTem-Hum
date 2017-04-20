#include <msp430g2553.h>
#include <stdio.h>
#include <intrinsics.h>

#define LED1 BIT0
#define LED2 BIT6
#define RX BIT1
#define TX BIT2
#define S2 BIT3
#define SDA BIT4    // P1.4
#define SCK BIT5    // P1.5

// Command SHT10
#define STATUS_REG_W 0x06   // binary: 0000 0110
#define STATUS_REG_R 0x07   // binary: 0000 0111
#define MEASURE_TEMP 0x03   // binary: 0000 0011
#define MEASURE_HUMI 0x05   // binary: 0000 0101
#define RESET      0x1e   // binary: 0001 1110

volatile unsigned int count = 0;
volatile unsigned int i;

void _config_clock(void)
{
    if(CALBC1_12MHZ == 0xFF)
        while(1);
    DCOCTL = 0;
    BCSCTL1 = CALBC1_12MHZ;
    DCOCTL = CALDCO_12MHZ;

    BCSCTL2 = SELM_0 | DIVM_0;
}

void _config_gpio(void)
{
    P1DIR = LED1 | LED2; // Set P1.0 to output direction
    P1OUT = S2;
    P1REN  = S2;
}

void _config_uart(void)
{
    P1SEL = RX | TX;            // P1.1 is RX, P1.2 is TX
    P1SEL2 = RX | TX;

    UCA0CTL1 = UCSWRST | UCSSEL_2;

    UCA0CTL0 = 0x00;
    UCA0MCTL = UCBRF_2 | UCBRS_0 | UCOS16;
    UCA0BR0 = 78;
    UCA0BR1 = 00;

    UCA0CTL1 &= ~UCSWRST;
    IE2 = UCA0RXIE;
    _BIS_SR(GIE);
}

void main(void)
{
    unsigned long a = 2352;
    unsigned long b = 186;

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    while(1)
    {


    }
}

void printf_long(unsigned long number)
{
    unsigned char buffer[16];
    unsigned int i, j;

    if (number == 0)
    {
        printf_char('0');
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
        printf_char(buffer[j]);
}
void printf_char(unsigned char m_char)
{
    while(!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = m_char;
}
void printf_string(char* string)
{
    for (i = 0; i< strlen(string); i++)
        printf_char(string[i]);
    _delay_cycles(12000000);
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_IRS (void)
{
    while(!(IFG2 & UCA0RXIFG));
    if (UCA0RXBUF == 'K')
        P1OUT ^= 0x01;
}
