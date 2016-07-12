#define Fosc            12000000                    //10MHz~25MHz
#define Fcclk           (Fosc * 5)                  //Fosc(1~32)<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO Fcclk 2񅦰�16�156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPB(Fcclk / 4) 1񄿔
#define  UART_BPS	9600 		//Change Baud Rate Setting here

unsigned char Temp=0,Temp2=0;

void Init_UART0(void);
void  __irq IRQ_UART0(void);
void UART0_SendByte(unsigned char data);
void UART0_SendStr(const unsigned char *str);

void Init_UART1(void);
void  __irq IRQ_UART1(void);
void UART1_SendByte(unsigned char data);
void UART1_SendStr(const unsigned char *str);



void Init_UART0(void)
{  
   unsigned int Baud16;
   PINSEL0&=0xFFFFFFF0;
   PINSEL0|=0x00000005;

   U0LCR = 0x83;		            // DLAB = 1
   Baud16 = (Fpclk / 16) / UART_BPS;  
   U0DLM = Baud16 / 256;							
   U0DLL = Baud16 % 256;						
   U0LCR = 0x03;
   U0IER = 0x00000001;		//Enable Rx interrupts

   VICIntSelect = 0x00000000;		// IRQ
   VICVectCntl0 = 0x20|6;			// UART0
   VICVectAddr0 = (int)IRQ_UART0; 	//UART0 Vector Address
   VICIntEnable = (1<<6);	// Enable UART0 Rx interrupt

}



void  __irq IRQ_UART0(void)
{  
 Temp = U0RBR;			
 VICVectAddr = 0x00;
 UART0_SendByte(Temp);	//Echo Back received character
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


 //This function is UART0 Receive ISR. This functions is called whenever UART0 receives any data

void Init_UART1(void)
{  
   unsigned int Baud16;
   PINSEL0&=0xFFF0FFFF;
   PINSEL0|=0x00050000;

   U1LCR = 0x83;		            // DLAB = 1
   Baud16 = (Fpclk / 16) / UART_BPS;  
   U1DLM = Baud16 / 256;							
   U1DLL = Baud16 % 256;						
   U1LCR = 0x03;
   U1IER = 0x00000001;		//Enable Rx interrupts

   VICIntSelect = 0x00000000;		// IRQ
   VICVectCntl0 = 0x20|7;			// UART1
   VICVectAddr0 = (int)IRQ_UART1; 	//UART1 Vector Address
   VICIntEnable = (1<<7);	// Enable UART1 Rx interrupt

}
				
//This function sends a single character on the serial port
void UART1_SendByte(unsigned char data)
{  
   U1THR = data;				    
   while( (U1LSR&0x40)==0 );	    
}
void UART1_SendByte_int(int data)
{  
   U1THR = data;				    
   while( (U1LSR&0x40)==0 );	    
}

void  __irq IRQ_UART1(void)
{  
 Temp2 = U1RBR;			
  
 
 //write code here
 VICVectAddr = 0x00;
 UART1_SendByte(Temp2);	//Echo Back received character

}

//This function sends a string of characters on the serial port
void UART1_SendStr(const unsigned char *str)
{  
   while(1)
   {  
      if( *str == '\0' ) break;
      UART1_SendByte(*str++);	    
  
   }


}
	