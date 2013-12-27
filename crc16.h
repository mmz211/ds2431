#include <stdint.h>

#ifndef _UTIL_CRC16_H_
#define _UTIL_CRC16_H_

#define POLYNOMIAL 0x1021
#define POLYNOMIALR 0x8408
uint16_t crc;


uint16_t crc16l(uint8_t *ptr,uint8_t len)
{
	uint8_t i;
	while (len--)
	{
		for(i=0x80;i!=0;i>>=1)
		{
			if((crc&0x8000)!=0) {crc<<=1;crc^=POLYNOMIAL;}
			else
				crc<<=1;
			if((*ptr&i)!=0) crc^=POLYNOMIAL;
		}
		ptr++;
	}
	return(crc);
}

uint16_t crc16r(uint8_t *ptr,uint8_t len)
{
	uint8_t i;
	while (len--)
	{
		for(i=0x01;i!=0;i<<=1)
		{
			if((crc&0x0001)!=0) {crc>>=1;crc^=POLYNOMIALR;}
			else
				crc>>=1;
			if((*ptr&i)!=0) crc^=POLYNOMIALR;
		}
		ptr++;
	}
	return(crc);
}
/*
uint16_t crc_ccitt_update(uint8_t *ptr,uint8_t len)
{
	while(len--)
    {
        *ptr ^= lo8 (crc);
        *ptr ^= *ptr << 4;

        return ((((uint16_t)*ptr << 8) | hi8 (crc)) ^ (uint8_t)(*ptr >> 4) 
                ^ ((uint16_t)*ptr << 3));
		ptr++;
    }
}*/

#endif /* _UTIL_CRC16_H_ */
