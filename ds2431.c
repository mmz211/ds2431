
#include "ds2431.h"

void delay_us(int _us)
{
	//int temp;
	if (_us<100)
		{_delay_us(_us);return;}
	else if(_us<200)
		{_delay_us(100);_delay_us(_us-100);return;}
	else if(_us<300)
		{_delay_us(100);_delay_us(100);_delay_us(_us-200);return;}
	else if(_us<400)
		{_delay_us(100);_delay_us(100);_delay_us(100);_delay_us(_us-300);return;}
	else if(_us<500)
		{_delay_us(100);_delay_us(100);_delay_us(100);_delay_us(100);_delay_us(_us-400);return;}
	else
		{_delay_us(100);_delay_us(100);_delay_us(100);_delay_us(100);_delay_us(100);_delay_us(_us-500);return;}
}

// delay in ms not very accurate
void delay_ms(int _ms)
{
	int temp;
	if(_ms<35)
		_delay_ms(_ms);
	else
		for(temp=_ms;temp>0;temp-=5)
		_delay_ms(5);
}

//send 'databyte' to 'port'
int outp1(int databyte)
{
	DDR=0xFF;
	OUT=databyte;
	return 1;
}

//read byte from 'port'
int inp1(void)
{
	int temp;
	DDR=0x00;
	return temp=IN;
}

// 'tick' values
int A,B,C,D,E,F,G,H,I,J;

// Set the 1-Wire timing to 'standard' (standard=1) or 'overdrive' (standard=0).

void SetSpeed(int standard)
{
	if (standard)
		{
		// Standard Speed
		A = 6;
		B = 64;
		C = 60;
		D = 10;
		E = 9;
		F = 55;
		G = 0;
		H = 550;
		I = 70;
		J = 360;
		}
	else
		{
		// Overdrive Speed
		A = 1.5;
		B = 7.5;
		C = 7.5;
		D = 2.5;
		E = 0.75;
		F= 7;
		G = 2.5;
		H = 70;
		I = 8.5;
		J = 40;
		}	
}

// Generate a 1-Wire reset, return 1 if no presence detect was found,
//return 0 otherwise.
// (NOTE: Does not handle alarm presence from DS2404/DS1994)
/*
int OWTouchReset(void)
{
	int result;

	TICK_DELAY(G);

	 //Drive DQ low
	 DDRB |= 0x10;
	 PORTB &= 0xEF;

	TICK_DELAY(H);

	 //Release the bus
	 PORTB |=0x10;
	 DDRB &= 0xEF;
	TICK_DELAY(I);

	result = PINB & 0x10; //Sample for presence pulse from slave

	TICK_DELAY(J);

	return result; //Return sample presence pulse result	
}
*/

#define T_RSTL		550
#define T_PHD			40
#define T_PDL			150
#define T_REC	 		5
#define T_W1L			8
#define T_W0L		90
#define T_MST		13


int OWTouchReset(void)
{
	int result;

	delay_us(0);

	SET_OUTPUT;	//DDRB |= 0x10;
	SET_PIN;				//PORTB &= 0xEF;
	delay_us(T_REC);

	CLR_PIN;
	delay_us(T_RSTL);

	SET_PIN;				// PORTB |=0x10;

	SET_INPUT;		// DDRB &= 0xEF;
	delay_us(T_PDL);

	// result = PINB & 0x10; //Sample for presence pulse from slave
	result = IN & BIT;

	delay_us(T_REC);//Complete the reset sequence recovery

	return result; //Return sample presence pulse result	
}

// Send a 1-Wire write bit. Provide 10us recovery time

void OWWriteBit(int bit)
{
	if(bit)
	{
		// Write '1' bit
		//Drive DQ low
	 	SET_OUTPUT;	//DDRB |= 0x10;
		SET_PIN;				//PORTB &= 0xEF;
		delay_us(2);

		CLR_PIN;				//PORTB &= 0xEF;
		delay_us(T_W1L);

		SET_PIN;				// PORTB |=0x10;
		SET_INPUT;		// DDRB &= 0xEF;
		delay_us(10);		// Complete the time slot and 10us recovery
	}

	else
	{
		//Write '0' bit
		//Drive DQ low
	 	SET_OUTPUT;
		SET_PIN;
		delay_us(2);

		CLR_PIN;	
		delay_us(T_W0L);

		//Release the bus
		SET_PIN;
		SET_INPUT;
		delay_us(10);
	}
}


// Read a bit from the 1-Wire bus and return it. Provide 10us recovery time.

int OWReadBit(void)
{
	int result;

	SET_OUTPUT;
	SET_PIN;
	delay_us(1);

	CLR_PIN;	
	delay_us(4);

	SET_PIN;
	SET_INPUT;
	delay_us(2);

	result = IN & BIT;
	delay_us(15);

	SET_OUTPUT;
	SET_PIN;

	return result;
}

// Write 1-Wire data byte

void OWWriteByte(int data)
{
	int loop;

	for (loop=0;loop<8;loop++)
	{
		OWWriteBit(data & 0x01);
		data >>=1;
	}
}

// Write 1-Wire data 2 bytes

void OWWrite2Byte(int data)
{
	int loop;

	for (loop=0;loop<16;loop++)
	{
		OWWriteBit(data & 0x01);
		data >>=1;
	}
}

// Read 1-Wire data type and return it

int OWReadByte(void)
{
	int loop;
	int result=0x00;

	for(loop=0;loop<8;loop++)
	{
		result >>= 1;
		if(OWReadBit())
			result |= 0x80;
	}
	return result;
}

// Read 1-Wire int type and return it

int OWRead2Byte(void)
{
	int loop;
	int result=0x00;
	for(loop=0;loop<16;loop++)
		{
		result >>= 1;
		if(OWReadBit())
			result |= 0x80;
		}
	return result;
}

// Write a 1-Wire data byte and return the sampled result

int OWTouchByte (int data)
{
	int loop,result=0;
	for (loop=0;loop<8;loop++)
		{
		result >>=1;

		if(data & 0x01)
			{
			if (OWReadBit())
				result |= 0x80;
			}
		else
			OWWriteBit(0);
		data >>=1;
		}
	return result;
}

// Write a block 1-Wire data bytes and return the sampled result in the same buffer.

void OWBlock (unsigned char *data,int data_len)
{
	int loop;
	for (loop=0;loop<data_len;loop++)
		{
		data[loop] = OWTouchByte(data[loop]);
		}
}

/*
*	Write Scratchpad
*
*	RST		|	PD	|	Select	|	WS	|	TA	|	<8-T2:T0 bytes>	|	CRC16\	|	FF loop	|
*
*/
int WriteScratchpad(unsigned int TA,unsigned char * data,int length)
{
	int crc;
	if(!OWTouchReset())
		{
		OWWriteByte(SKIPROM);
		OWWriteByte(WS);
		OWWrite2Byte(TA);
		OWBlock(data, length);
		OWReadByte();
		OWReadByte();
		crc=OWRead2Byte();
		return crc;
		}
	else
		return 0;
}

/*
*	Read Scratchpad
*	TA1,TA2,ES,Data is returned via pointer
*
*	RST		|	PD	|	Select	|	RS	|	TA-E/S	|	<8-T2:T0 bytes>	|	CRC16\	|	FF loop	|
*
*/
int ReadScratchpad(unsigned int *TA,char *ES,char * data,int length)
{
	int i,crc;
	if(!OWTouchReset())
		{
		OWWriteByte(SKIPROM);
		OWWriteByte(RS);
		*TA=OWReadByte();
		*TA<<=8;
		*TA|=OWReadByte();
		*ES=OWReadByte();
		for(i=0;i<length;i++)
			data[i]=OWReadByte();
		crc=OWRead2Byte();
		return crc;
		}
	else
		return 0;
}

/*
*	Copy Scratchpad
*
*	RST		|	PD	|	Select	|	CPS	|	TA-E/S	|	Programming	|	AA loop	|
*
*/
int CopyScratchpad(unsigned int TA,char ES)
{
	if(!OWTouchReset())
		{
			OWWriteByte(SKIPROM);
			OWWriteByte(CPS);
			OWWrite2Byte(TA);
			OWWriteByte(ES);
			_delay_ms(12.5);
			if(OWReadByte()==0xAA)
				return SUCCESS;
			else
				return FAIL;
		}
	else
		return FAIL;
}

/*
*	Read Memory (length < BUFFLELEN)
*
*	RST		|	PD	|	Select	|	RM	|	TA	|	<data to EOM>	|	FF loop	|
*
*/

char *ReadMemory(unsigned int TA,int length)
{
	int i;
	if(!OWTouchReset())
		{
led_dis(5);

			OWWriteByte(SKIPROM);
			OWWriteByte(RM);
			OWWrite2Byte(TA);
led_dis(4);

			ClearBuffle();
			for(i=0;i<length;i++)
				ReadMemoryBuffle[i]=OWReadByte();
led_dis(6);

			OWTouchReset();
led_dis(3);

			return ReadMemoryBuffle;
		}
	else
		return 0;
}

void ClearBuffle(void)
{
	int i;
	for(i=0;i<BUFFLELEN;i++)
		ReadMemoryBuffle[i]=0x0A;
}

