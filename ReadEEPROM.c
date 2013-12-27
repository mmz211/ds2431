#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "ds2431.h"
#include "led.h"
#include "crc16.h"


#define fosc 7372800UL 									//??7.3728MHZ
#define baud 9600										//???

char * data;

void uart0_init(void)
{

     UCSR0B = 0x00; 							//disable while setting baud rate
     UCSR0A = 0x00;
     UCSR0C =(1<<UCSZ01)|(1<<UCSZ00);			//8bit+1bit stop
     UBRR0L = (fosc/baud/16-1)%256;									//(fosc/16/(baud+1))%256;
     UBRR0H = (fosc/baud/16-1)/256;									//(fosc/16/(baud+1))/256;
     UCSR0B =(1<<RXEN0)|(1<<TXEN0);			//RXCEN TXCEN
}

void uart1_init(void)
{

     UCSR1B = 0x00; 							//disable while setting baud rate
     UCSR1A = 0x00;
     UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);			//8bit+1bit stop
     UBRR1L = 0x2F;                             //(fosc/baud/16-1)%256;								//(fosc/16/(baud+1))%256;
     UBRR1H = 0x00;  							//(fosc/baud/16-1)/256;								//(fosc/16/(baud+1))/256;
     UCSR1B =(1<<RXEN1)|(1<<TXEN1);			//RXCEN TXCEN
}

void putchar0(unsigned char c)
{ 
    while (!(UCSR0A&(1<<UDRE0)));
    UDR0=c;   
}


void putchar1(unsigned char c)
{ 
    while (!(UCSR1A&(1<<UDRE1)));
    UDR1=c;   
}


unsigned char getchar0(void)
{
    while(!(UCSR0A& (1<<RXC0)));
    return UDR0;
}


unsigned char getchar1(void)
{
    while(!(UCSR1A& (1<<RXC1)));
    return UDR1;
}


void puts0(char *s)
{
    while (*s)
    {
        putchar0(*s);
        s++;
    }
    putchar0(0x0a);
    putchar0(0x0d);
}


void puts1(char *s)
{
    while (*s)
    {
        putchar1(*s);
        s++;
    }
    putchar1(0x0a);
    putchar1(0x0d);
}


int main(void)
{
	led_dis(0); //ON

	SetSpeed(1);

	__asm__ __volatile__("sei" ::);
	uart0_init();
	//data = ReadMemory(0x00,128);
	data = ReadMemory(0x00,128);
	puts0(data);

/*
	address=0;

	for(i=length+2;i>0;i-=8)
		{
		WriteScratchpad(address,data+address,8);
		ReadScratchpad(TA,ES,databuffle,length+2);
		CopyScratchpad(address,0x07);
		address+=8;
		}


DDRB |= 0x10;

char i;

DDRB |= 0x10;

for(i=0;i<1000;i++)
{
	 PORTB &= 0xEF;
		delay_us(550);//delay_us(1000);//delay_ms(500);
	 PORTB |= 0x10;
		delay_us(550);//delay_us(1000);//delay_ms(500);
}
*/
	//Notification();		//Notify when uploading completes
}


/*

//send 'databyte' to 'port'
int outp1(int databyte)
{
	DDRB=0xFF;
	PORTB=databyte;
	return 1;
}

//read byte from 'port'
int inp1(void)
{
	int temp;
	DDRB=0x00;			// input direction
	return temp=PINB;
}

*/

