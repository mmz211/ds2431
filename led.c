#include "led.h"
#include "ds2431.h"

/*
* Toggle red led
*/
void toggle_red(void)
{
	int temp;
	DDRA&=0xFD;
	temp=PINA;
	temp^=0x02;
	DDRA|=0x02;
	PORTA=temp;
	
}


void redOn(void)
{
	led_dis(5);
}

/*
*  Display the lowest three bits on leds
*/

void led_dis(int dig)
{
	DDRA|=0x07;
	PORTA=((unsigned char)dig);
}


/*
*	Display the lower 9 bits on leds (start with a flash)
*/
void Dig2Led(int dig)
{

	led_dis(dig);
	_delay_ms(500);
	led_dis(dig>>3);
	_delay_ms(500);
	led_dis(dig>>6);
	_delay_ms(500);
}
