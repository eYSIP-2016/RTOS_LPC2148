#include <LPC214x.h>
#include "uart0.h"


/************************************************************

	Function 		: Init_UART0
	Return type		: None
	Parameters		: None
	Description 	: Initialises UART0 module. 


************************************************************/
void Init_UART0(void)
{  
   unsigned int Baud16;
   PINSEL0&=0xFFFFFFF0;
   PINSEL0|=0x00000005;

   U0LCR = 0x83;		            // DLAB = 1
   //Baud16 = (Fpclk / 16) / UART_BPS;  
   //U0DLM = 0x00;//Baud16 / 256;	Baud rate =116071 	D=8, M=13					
   //U0DLL = 0x05;//Baud16 % 256;	
   //U0FDR = 0xD8;	//D=8, M=13  Refer Datasheet for more details, use debugger in simulator mode to calculate baudrate					
   Baud16 = (Fpclk / 16) / UART_BPS;  
   U0DLM = Baud16 / 256;							
   U0DLL = Baud16 % 256;						
   U0LCR = 0x03;
   U0IER = 0x00000001;		//Enable Rx interrupts

   VICIntSelect = 0x00000000;		// IRQ
   VICVectCntl2 = 0x20|6;			// UART0
   VICVectAddr2 = (int)IRQ_UART0; 	//UART0 Vector Address
   VICIntEnable = (1<<6);	// Enable UART0 Rx interrupt

}
				

//This function sends a single character on the serial port
void UART0_SendByte(unsigned char data)
{  
   U0THR = data;				    
   while( (U0LSR&0x40)==0 );	    
}

//This function sends a string of characters on the serial port
void UART0_SendStr(const unsigned char *str)
{  
   while(1)
   {  
      if( *str == '\0' ) break;
      UART0_SendByte(*str++);	    
   }
}
