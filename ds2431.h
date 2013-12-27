#include <avr/io.h>
#include <util/delay.h>

#include "led.h"

#ifndef _DS2431_H_
#define _DS2431_H_ 1

void delay_us(int _us);

// delay in ms not very accurate
void delay_ms(int _ms);

// PORT

#define F_CPU 7372800UL

#define DDR							DDRB
#define IN								PINB
#define OUT							PORTB
#define BIT							0x10
#define BIT_SHIFT			4

#define SET_OUTPUT 	 	DDR |= BIT
#define SET_INPUT			DDR &= ~BIT
#define SET_PIN					OUT |= BIT
#define CLR_PIN					OUT &= ~BIT

// --PORT

#define OUTPUT_H 	 		DDR |= BIT


enum
{
	SUCCESS=1,
	FAIL=0
};

#define TICK_DELAY(tick) (_delay_us((double)tick))

#define BUFFLELEN 128

char ReadMemoryBuffle[BUFFLELEN];	//Create Buffle for memory reading

/* 
*	One-wire Communication Protocol:
*	RST:				Reset
*	PD:					Reply from ds2431
*	Select:				Commands
*	WS:					Write Scratchpad
*	RS:					Read Scratchpad
*	CPS:				Copy Scratchpad
*	RM:					Read Memory
*	TA:					Target Address TA1,TA2
*	TA-E/S:				Target Address TA1,TA2,E/S
*	<8-T2:T0 bytes>:		Enough bytes to reach the end of the row (8 minus [T2:T0])
*	<data to EOM>:		Read enough bytes to reach the end of the row
*	CRC16\:				Reverse CRC16
*	FF loop:				Read FF uncertain loop
*	AA loop:				Read AA uncertain loop
*	Programming:			Data writing to EEPROM,no activity on wire's allowed
*
*/

// define rom commands
#define READROM				0x33	//Read Rom, Only one device attached
#define MATCHROM			0x55	//Match Rom
#define SEARCHROM			0xF0	//Search Rom
#define SKIPROM				0xCC	//Commonly used when only one device's attached
#define RESUME					0xA5	//Resume
#define ODSKIPROM			0x3C	//Overdrive Skip Rom
#define ODMATCHROM	0x69	//Overdrive Match Rom


// define WS,RS,CPS,RM
#define WS							0x0F		//Write Scratchpad
#define RS								0xAA		//Read Scratchpad
#define CPS							0x55		//Copy Scratchpad
#define RM							0xF0		//Read Memory

//	Clear Read Buffle;
void ClearBuffle(void);

// Set the 1-Wire timing to 'standard' (standard=1) or 'overdrive' (standard=0).

void SetSpeed(int standard);

/*
*	Write Scratchpad
*
*	RST		|	PD	|	Select	|	WS	|	TA	|	<8-T2:T0 bytes>	|	CRC16\	|	FF loop	|
*
*/
int WriteScratchpad(unsigned int TA,unsigned char * data,int length);

/*
*	Read Scratchpad
*	TA1,TA2,ES,Data is returned via pointer
*
*	RST		|	PD	|	Select	|	RS	|	TA-E/S	|	<8-T2:T0 bytes>	|	CRC16\	|	FF loop	|
*
*/
int ReadScratchpad(unsigned int *TA,char *ES,char *data,int length);

/*
*	Copy Scratchpad
*
*	RST		|	PD	|	Select	|	CPS	|	TA-E/S	|	Programming	|	AA loop	|
*
*/
int CopyScratchpad(unsigned int TA,char ES);

/*
*	Read Memory
*
*	RST		|	PD	|	Select	|	RM	|	TA	|	<data to EOM>	|	FF loop	|
*
*/
char *ReadMemory(unsigned int TA,int length);


// Generate a 1-Wire reset, return 1 if no presence detect was found,
//return 0 otherwise.
// (NOTE: Does not handle alarm presence from DS2404/DS1994)

int OWTouchReset(void);

// Send a 1-Wire write bit. Provide 10us recovery time

void OWWriteBit(int bit);

// Read a bit from the 1-Wire bus and return it. Provide 10us recovery time.

int OWReadBit(void);

// Write 1-Wire data byte

void OWWriteByte(int data);

// Write 1-Wire data 2 bytes

void OWWrite2Byte(int data);

// Read 1-Wire data type and return it

int OWReadByte(void);

// Read 1-Wire int type and return it

int OWRead2Byte(void);

// Write a 1-Wire data byte and return the sampled result

int OWTouchByte (int data);

// Write a block 1-Wire data bytes and return the sampled result in the same buffer.

void OWBlock (unsigned char *data,int data_len);

#endif   // _DS2431_H_
