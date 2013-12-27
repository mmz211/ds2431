//***************************************************************************
// Communicate with eKo node (1/2 duplex)
//*************************************************************************** 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "global.h"
#include "eko_msg.h"
#include "comm_eko.h"

//bPowerAlwaysOn set by sensor_xx.c file
extern bool   bPowerAlwaysOn;

static bool bEkoMsgRcvd;        	//True if eko message rcvd
static bool bWaiting4TimeOut;    	//True if RS485 rcv enabled & waiting for eko msg
static uint8* eko_data;        		//pntr to eko data pkt
static uint8 eko_data_indx;
static bool bReceiving;
static bool bPktEsc; 
static bool bFlagByte;

/******************************************************************************
* crcByte : CRC-16 function produces a 16-bit running CRC 
 * The ITU-T polynomial is: G_16(x) = x^16 + x^12 + x^5 + 1
******************************************************************************/
uint16 crcByte(uint16 crc, uint8 b){

  uint8 i;

  crc = crc ^ b << 8;
  i = 8;
  do
    if (crc & 0x8000)
      crc = crc << 1 ^ 0x1021;
    else
      crc = crc << 1;
  while (--i);

  return crc;
}
/******************************************************************************
  FUNCTION: HDLC_calcCRC
   DESCRIPTION: Calculate 16bit CRC over specified number of bytes
   RETURNS: 16bit CRC
******************************************************************************/
uint16 HDLC_calcCRC(uint8 * ptr, uint8 nofbytes) {

    uint16 crc;
    crc = 0;

    while (nofbytes-- > 0)
      crc = crcByte(crc, *ptr++);

    return crc;
}

/******************************************************************************
  enable_Timer1 :
   Timer1 used to disable RS485 is no eko msg rcvd.
   Not used unless sensor requires eko power to be always on
 *****************************************************************************/
void enable_Timer1(){

  if (!bPowerAlwaysOn) return;
  // TCCR1B_Bit0 = 1;         // clk/1024 
  // TCCR1B_Bit2 = 1;         // clk/1024
  TCCR1B |= 0x05;			  // clk/1024
  TCNT1H = 0;              	//zero timer
  TCNT1L = 0; 
  TIMSK |= 4;          		//interrupt on overflow 
}

/******************************************************************************
  disable_Timer1 :
   Timer1 used to disable RS485 is no eko msg rcvd.
 *****************************************************************************/
void disable_Timer1(void){
  // TCCR1B_Bit0 = 0;         // clk off 
  // TCCR1B_Bit2 = 0;         // clk off
  TCCR1B &= 0xF8;				// clk off
  TCNT1H = 0;       	        //zero timer
  TCNT1L = 0; 
  TIMSK &= 0xFB;		          	//disable interrupt on overflow 
}

/******************************************************************************
disable eko Timer1 interrupt
 -Not used unless sensor requires eko power to be always on
******************************************************************************/
void disable_eko_timer_int(void){

   	if (!bPowerAlwaysOn) 
		return;

   	bWaiting4TimeOut = FALSE;

   	disable_Timer1();

   	return; 
}  
/******************************************************************************
* comm_eko_init
* - initialize uart0 for 1/2 duplex comm with eKo
* - enable RS485 receiver
* - get pntr to eko data bfr
* - enable eko node interrupt
* - start Timer1. If expires before any eKo communication then disable RS485
******************************************************************************/
void comm_eko_init(uint8* eko_pkt){

  	uint8 data;	
  	data = UDR; 							//clear input rcvr        

  	eko_data = eko_pkt; 

    UCSRB = 0x00;                     		//disable while setting baud rate
    UCSRA = 0x00;
 
	// Character Size=8-bit, all other bits are default
  	UCSRC = (1 << UCSZ1) | (1 << UCSZ0);

	//set baud rate = 19200 for 7.3728Mhz crystal  
  	UBRRH = 0;  
  	UBRRL = 23;

  	UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << RXCIE); 

	//configure RS485 control pins 
  	sbi(DDRB,PB2);
	cbi(PORTB,PB2);

	//Init Timer1 - if no eko comm before timeout, disable RS485 to conserve pwr
  	//enable_Timer1();

  	eko_data_indx = 0;
  	bReceiving = TRUE;
  	bEkoMsgRcvd = FALSE;
  	bWaiting4TimeOut = TRUE;
  	bPktEsc = FALSE; 
  	bFlagByte = FALSE;

	return;
}
/******************************************************************************
void eko_init_rcv(void)
 -reset params
 - enable RS485 rcv; 
******************************************************************************/
void eko_init_rcv(void){

  	uint8 data;
  
  	eko_data_indx = 0;
  	bReceiving = TRUE;
  	bEkoMsgRcvd = FALSE;
  	bPktEsc = FALSE; 
  	bFlagByte = FALSE;

  	data = UDR;                  	// clear input rcvr

  	cbi(PORTB,PB2);					// enable rcv, disable xmit

  	return;
}
/******************************************************************************
void uart_RS485_xmit(void)
 eko msg rcvd - xmit resonse
 disable RS485 rcv/ enable xmit
 wait for 1 byte delay to allow 1/2 duplex switching glitches  
******************************************************************************/
void uart_RS485_xmit(void){
  	sbi(PORTB,PB2);					// enable xmit, disable rcv
}
/******************************************************************************
rs485_disable() disable RS485 receiver
******************************************************************************/
void rs485_disable(void){

  	uint8 data;

  	cbi(DDRB,PB2);					// disable rcv, disable xmit

  	eko_data_indx = 0;
  	bReceiving = TRUE;
  	bEkoMsgRcvd = FALSE;
  	bPktEsc = FALSE; 
  	bFlagByte = FALSE;

  	data = UDR;                		//clear input rcvr        

  	sbi(DDRB,PB2);					
  	cbi(PORTB,PB2);					// enable rcv, disable xmit

	return;
}  
/******************************************************************************
rs485_waiting() : return:
- 0 if RS485 not enabled.
- 1 if  "    enabled and eKo msg has been received
- 2 if  "    enabled and waiting for eKo msg
******************************************************************************/
uint8 rs485_waiting(void)
{

 	if (!bWaiting4TimeOut)
		return 0;
 	if (bWaiting4TimeOut && bEkoMsgRcvd) 
		return 1;
 	if (bWaiting4TimeOut && !bEkoMsgRcvd) 
		return 2;

 	return 0;
}  

/******************************************************************************
xmitChar
  -xmit char from uart
******************************************************************************/
void xmitChar(uint8 data) {

	while(!(UCSRA & (1 << UDRE)));		//stuck until uart xmit reg empty
    UDR = data;                  		//xmit the data

   	return;
}

/******************************************************************************
 xmitData(uint8 data)
  - xmit data
  - If reserved code expand to 0x7D and escaped char
******************************************************************************/  
void xmitData(uint8 data) {

    if (data == HDLC_CTLESC_BYTE || data == HDLC_FLAG_BYTE) {
      	//expand to ESCAPE sequence
		xmitChar( (unsigned int)HDLC_CTLESC_BYTE );
		xmitChar( (unsigned int)data ^ 0x20 );   //"escaped" data
    }
    else 
		xmitChar( (unsigned int)data );   		//tx the data
    
    return;
}	

/******************************************************************************
xmitDelimiter
  -xmit delimiter char
******************************************************************************/
void xmitDelimiter(void) {
	xmitChar( (unsigned int)HDLC_FLAG_BYTE );
}

/******************************************************************************
XmitEkoPkt
  -xmit eko pkt back to node
  -delay before return in case DIM100 is always powered and needs to go back
   into 1/2 duplex rcv mode. Otherwise eKo will just turn off power to DIM100
   after msg sent.
******************************************************************************/
void XmitEkoPkt(uint8 *pkt_data){

    uint16 crc,i;
    uint8 nob,counter;
 
    nob = pkt_data[0]+1;      						//# of data bytes to xfr
    crc = HDLC_calcCRC(pkt_data,nob);  				//don't inlcude crc
 	//send pkt
    xmitDelimiter();   								//start a frame
    for(counter = 0; counter < nob; counter++){
   		xmitData(pkt_data[counter]);
   	}
   	xmitData(crc & 0xff);
   	xmitData((crc >> 8) & 0xff);
   	xmitDelimiter();								//end a frame
   	// while (UCSR0A_Bit5 == 0){};   				//stuck until uart xmit reg empty
   	while(!(UCSRA & (1 << UDRE)));
   
   	for (i = 0; i<= 50;i++){
   		xmitChar(0);
	}										   		//xmit nulls 

   return;
}

/******************************************************************************
//interrupt handler for incoming serial eKo data
//data packet starts and ends with HDLC_FLAG_BYTE (0x7e) 
//if data = 0x7e or 0x7d then data is 'escaped'
******************************************************************************/
SIGNAL(SIG_UART_RECV){

     uint16 crc,crc_chk;
     uint8 uart_data;

     uart_data = UDR;

     switch (uart_data) {
         case HDLC_FLAG_BYTE:
            if (eko_data_indx == 0)
				bFlagByte = TRUE;
            if (eko_data_indx != 0)
				bReceiving = FALSE;    					//end of packet
            break;
         case HDLC_CTLESC_BYTE:
             bPktEsc = TRUE;
             break;  
         default:
           	if(bFlagByte){  
             	if (bPktEsc)
               		eko_data[eko_data_indx++] = uart_data ^ 0x20;
             	else{
               		eko_data[eko_data_indx++] = uart_data;
               		bPktEsc = FALSE;
             	}
           }
           if (eko_data_indx >= EKO_MAX_MSG_SIZE)
		   		eko_data_indx =0; 						//Over run 
     }; //switch

     if(!bReceiving){                                //data pkt rcvd; chk crc

      	crc_chk = HDLC_calcCRC(&eko_data[0], eko_data_indx - 2);

      	crc = (eko_data[eko_data_indx-1] << 8) | eko_data[eko_data_indx - 2];

      	if (crc_chk == crc)
        	bEkoMsgRcvd = TRUE;                          //valid msg
     }

	 return;
}

/******************************************************************************
Timer1 interrupt handler
 - Timer 1 fired - no eKo communication
 - Disable RS485
 - Disable Timer1 interrupt
*******************************************************************************/
SIGNAL(SIG_OVERFLOW1){

  	if (!bPowerAlwaysOn){
    	disable_Timer1();   
    	return;
  	}
  
   	bWaiting4TimeOut = FALSE; 

   	cbi(DDRB,PB2);										//disable rcv, disable xmit
   	disable_Timer1();   

   	return;
}
