#include <msp430g2553.h>
#include <stdio.h>
#include <intrinsics.h>     //__no_operation();

#define LED1 BIT0   // P1.0
#define RX BIT1     // P1.1
#define TX BIT2     // P1.2
#define S2 BIT3     // P1.3
#define SDA BIT4    // P1.4
#define SCK BIT5    // P1.5
#define LED2 BIT6   // P1.6

// Initial Command for SHT10
#define STATUS_REG_W 0x06   // binary: 0000 0110
#define STATUS_REG_R 0x07   // binary: 0000 0111
#define MEASURE_TEMP 0x03   // binary: 0000 0011
#define MEASURE_HUMI 0x05   // binary: 0000 0101
#define RESET      0x1e   // binary: 0001 1110

// Initial state for SCK and SDA
#define SCK_H P1OUT|=SCK    // SCK High
#define SCK_L P1OUT&=~SCK   // SCK Low
#define SDA_H P1OUT|=SDA    // SDA High
#define SDA_L P1OUT&=~SDA   // SDA Low

#define SDA_out P1DIR|SDA      // set SDA to output
#define SDA_in P1DIR&=~SDA    // set SDA to input

#define NOACK 0                // no acknowledge
#define ACK 1                   // acknowledge
#define TEMPERATURE 1           // Temperature mode
#define HUMIDITY 2              // humidity mode

// 12 bit Humidity and 14 Bit Temperature
const float C1 = -2.0468;           // for 12 Bit Humidity
const float C2 = 0.0367;            // for 12 Bit Humidity
const float C3 = -0.0000015955;     // for 12 Bit Humidity
const float T1 = 0.01;              // for 12 bit Humidity
const float T2 = 0.00008;           // for 12 bit Humidity
const float D1 = -39.6;             // for 3V
const float D2 = 0.01;              // for 14 Bit Temperature

// Function calculate temperature and humidity
void SHT10_calculate(float *f_temperature, float *f_humidity)
{
    *f_temperature = D1 + D2*(*f_temperature);
    *f_humidity = (*f_temperature - (float) 25)*(T1 + T2*(*f_humidity)) + (C1 + C2*(*f_humidity) + C3*(*f_humidity)*(*f_humidity));
}
void _config_clock(void)
{
    if(CALBC1_1MHZ == 0xFF)
        while(1);
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    BCSCTL2 = SELM_0 | DIVM_0;
}

void _config_gpio(void)
{
    P1SEL = RX | TX;            // P1.1 is RX, P1.2 is TX
    P1SEL2 = RX | TX;
    P1DIR = LED1 | LED2 | SCK;  // Set P1.0, P1.6 and P1.5 to output direction
}

void SHT10_transmission_start()
{
    SDA_out;                 // set SDA to output
    SDA_H;
    SCK_L;
    __no_operation();
    SCK_H;
    __no_operation();
    SDA_L;
    __no_operation();
    SCK_L;
    __no_operation();
    __no_operation();
    __no_operation();
    SCK_H;
    __no_operation();
    SDA_H;
    __no_operation();
    SCK_L;
    SDA_in;            // set SDA to input
}
// Function reset connection
void SHT10_reset_connection()
{
    unsigned int i;
    SDA_out;          //Change SDA to output
    SDA_H;
    SCK_L;
    for(i=0;i<9;i++)       //9 SCK cycles
    {
        SCK_H;
        SCK_L;
    }
}
unsigned char SHT10_ReadData(unsigned int ack)
{
    // reads a byte form the Sensibus and gives an acknowledge in case of "ack=1"
    unsigned char i, val = 0;
    SDA_out;                      //Change SDA to be output 0:input 1:ouput
    SDA_H;                           //release DATA-line
    SDA_in;                     //Change SDA to be input 0:input 1:ouput
    for(i=0x80;i>0;i/=2)             //shift bit for masking
    {
        SCK_H;                     //clk for SENSI-BUS
        if(P1IN&SDA)
          val = val | i;             //read bit
        SCK_L;
    }
    SDA_out;                      //Change SDA to output
    if(ack)                        //in case of "ack==1" pull down DATA-Line
        SDA_L;
    else
        SDA_H;
    SCK_H;                           //clk #9 for ack
    __no_operation();
    __no_operation();
    __no_operation();               //pulswith approx. 5 us
    SCK_L;
    SDA_H;                           //release DATA-line
    SDA_in;                         //Change SDA to input
    return val;
}
unsigned int SHT10_WriteByte(unsigned char value)
{
    // writes a byte on the Sensibus and checks the acknowledge
    unsigned char i;
    unsigned int error = 0;
    SDA_out;
    for(i=0x80;i>0;i/=2)            //shift bit for masking
    {
        if(i&value)
            SDA_H;            //masking value with i , write to SENSI-BUS
        else SDA_L;
          SCK_H;                      //clk for SENSI-BUS
          __no_operation();
          __no_operation();
          __no_operation();     //pulswith approx. 5 us
          SCK_L;
    }
    SDA_H;                            //release DATA-line
    SDA_in;                      //Change SDA to be input 0:input 1:ouput
    SCK_H;                            //clk #9 for ack
    error = P1IN & SDA;              //check ack (DATA will be pulled down)
    SDA_out;                     //Change SDA to be output 0:input 1:ouput
    SCK_L;
    if(error)
        return 1;                     //error = 1 in case of no acknowledge
    return 0;
}
unsigned int SHT10_Measure(unsigned char *ch_value, unsigned char *ch_checksum, unsigned int ch_mode)
{
    unsigned int error = 0;
    unsigned int i;

    SHT10_transmission_start();                   //transmission start
    switch(ch_mode)
    {                                 //send command to sensor
        case TEMPERATURE:
            error += SHT10_WriteByte(MEASURE_TEMP);
            break;
        case HUMIDITY:
            error += SHT10_WriteByte(MEASURE_HUMI);
            break;
    }
    SDA_in;                      //Change SDA to input
    for(i=0;i<65535;i++)
        if((P1IN&SDA) == 0)
            break;                     //wait until sensor has finished the measurement
    if(P1IN&SDA)
        error += 1;                         //or timeout (~2 sec.) is reached
    *ch_value= SHT10_ReadData(ACK);         //read the first byte (MSB)
    *(ch_value + 1)= SHT10_ReadData(ACK);   //read the second byte (LSB)
    *ch_checksum = SHT10_ReadData(NOACK);      //read checksum
    return error;
}
void _config_uart(void)
{
    UCA0CTL1 = UCSWRST | UCSSEL_2;

    UCA0CTL0 = 0x00;
    UCA0MCTL = UCBRF_8 | UCBRS_0 | UCOS16;
    UCA0BR0 = 6;
    UCA0BR1 = 00;

    UCA0CTL1 &= ~UCSWRST;
    IE2 = UCA0RXIE;
    _BIS_SR(GIE);
}
void printf_int(unsigned long number)
{
    unsigned char buffer[16];
    unsigned int i, j;

    if(number < 0)
        printf_char('-');
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
void printf_string(unsigned char* string)
{
    unsigned int i;
    for (i = 0; i< strlen(string); i++)
        printf_char(string[i]);
}

void printf_float(float m_float, unsigned int m_int)
{
    unsigned long temp;
    float x;
    if (m_float < 0)
    {
        printf_char('-');
        m_float *= -1;
    }
    temp = (unsigned long) m_float;
    printf_int(temp);
    x = m_float - (float)temp;
    while(m_int > 0)
    {
        x *= 10;
        m_int --;
    }
    printf_char('.');
    x = (unsigned long)(x + 0.5);   // Lam tron
    printf_int(x);
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_IRS (void)
{
    while(!(IFG2 & UCA0RXIFG));
    if (UCA0RXBUF == 'K')
        P1OUT ^= 0x01;
}
void main(void)
{
    unsigned int unint_humidity;
    unsigned int unint_temperature;
    float f_humidity;
    float f_temperature;

    unsigned int unint_temperature_high;
    unsigned int unint_temperature_low;
    unsigned int unint_humidity_high;
    unsigned int unint_humidity_low;

    unsigned char unchar_checksum;
    unsigned int error = 0;

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    _config_clock();
    _config_gpio();
    _config_uart();
    while(1)
    {
        SHT10_reset_connection();
        error += SHT10_Measure((unsigned char) &unint_temperature, &unchar_checksum, TEMPERATURE);
        error += SHT10_Measure((unsigned char) &unint_temperature, &unchar_checksum, HUMIDITY);
        if(error != 0)
            SHT10_reset_connection();
        else
        {
            unint_humidity_high = (unint_humidity & 0x000f) << 8;
            unint_humidity_low = (unint_humidity & 0xff00) >> 8;
            unint_humidity = unint_humidity_high + unint_humidity_low;
            unint_temperature_high = (unint_temperature & 0x000f) << 8;
            unint_temperature_low = (unint_temperature & 0xff00) >> 8;
            unint_temperature = unint_temperature_high + unint_temperature_low;

            f_temperature = (float)unint_temperature;
            f_humidity = (float)unint_humidity;
            SHT10_calculate(&f_temperature, &f_humidity);

            printf_string("Temperature: ");
            printf_float(f_temperature, 3);
            printf_string(" oC, ");
            printf_string("Humidity: ");
            printf_float(f_humidity, 3);
            printf_string(" %\n");
        }
    }
}
