/*

Replaced contenets in lpc21xx with that of lpc214x to remove warnings 
Include startup.s from experiments in case of data abort
Counting.axf: Error: L6218E: Undefined symbol pvPortMalloc (referred from tasks.o). error is caused when heap 2 is not included
*/

/************************************************************************ 
Written by: K V S SUMAKAR and KARTIKEYAN V, ERTS Lab, IIT Bombay. Date: 23 June 2016
 IDE: Keil uVision4


Problem:To Illustrate the concept of MailBox using FreeRTOS
Concepts covered:	
MailBox

 Note:
 
 1. Do don't use the default Startup.s	provided by Keil. 
 	Instead follow the procedure given below :
	a. Copy the Startup.S file from any example project into the folder containing the project.
	b. Right click on the group icon in the project window and click on 'Add files to Group <group name>'
	c. Select and add the Startup.s file which was previously added.

 2. Make sure that in the Target options following settings are 
 	done for proper operation of the code

 	Device -> Microcontroller	: LPC2148
 	Target -> Frequency			: 12 Mhz
 	Output -> Create Hex File	: Checked (For more information read section 4.3.1 "Setting up Project in Keil uVision" in the hardware manual)
	C/C++  -> Include paths 	: ..\..\Source\include;..\..\Source\portable\RVDS\ARM7_LPC21xx;..\<project-folder-name>
	ASM    -> Include paths 	: ..\..\Source\portable\RVDS\ARM7_LPC21xx


 3. Ensure that following settings are done in Startup.s configuration wizard:

 	Clock Settings:
		
	PLL Steup	 >>	 MSEL=5, PSEL=2
	VPBDIV Setup >>  VPBCLK = CPU Clock
	
 4. Include the following FreeRTOS C files in the project either in the same group, or by creating a new group in the project.
	
	 ( These will be available in the FreeRTOS source files. )
	
	FILE		SOURCE (may change according to location of FreeRTOS source code)
	list.c	 	FreeRTOSv8.0.1\FreeRTOS\Source\list.c
	queue.c		FreeRTOSv8.0.1\FreeRTOS\Source\queue.c
	tasks.c		FreeRTOSv8.0.1\FreeRTOS\Source\tasks.c
	heap_2.c	FreeRTOSv8.0.1\FreeRTOS\Source\portable\MemMang\heap_2.c
	port.c		FreeRTOSv8.0.1\FreeRTOS\Source\portable\RVDS\ARM7_LPC21xx\port.c
	portASM.s	FreeRTOSv8.0.1\FreeRTOS\Source\portable\RVDS\ARM7_LPC21xx\portASM.s
	
 5. Copy FreeRTOSConfig.h file into	the project folder. Configure it according to the application. 
 	use this link for reference : http://www.freertos.org/a00110.html
 
*/
//****************************************************************************************************/
/* 
Note: To use mutex semaphore you need to initialize  configUSE_MUTEXES to 1 
*/
//configUSE_TASK_NOTIFICATIONS to 1

#include<stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include"semphr.h"
//#incldue<math.h>
TaskHandle_t xHandle = NULL;

char Temp;
//UART0 declarations for serial communication
#define Fosc            12000000                    //10MHz~25MHz
#define Fcclk           (Fosc * 5)                  //Fosc(1~32)<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO Fcclk 2񅦰�16�156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPB(Fcclk / 4) 1񄿔
#define  UART_BPS	9600 		//Change Baud Rate Setting here
#define CR     0x0D


void DelaymSec(unsigned int j);
void Init_Peripherals(void);
void Init_Ports(void);
void  __irq IRQ_UART0(void);
void Init_UART0(void);
void UART0_SendByte(unsigned char data);	//For sending a byte of Data through serial monitor
void UART0_SendStr(const unsigned char *str);//For sending a string of Data through serial monitor


//This function is UART0 Receive ISR. This functions is called whenever UART0 receives any data
void  __irq IRQ_UART0(void)
{  
 Temp = U0RBR;			
 
 VICVectAddr = 0x00;
 UART0_SendByte(Temp);	//Echo Back received character
}		

// Initialises UART0 module. 
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
				

//This function sends a single character on the serial port

void UART0_SendByte(unsigned char data)
{  	
   U0THR = data;				    
   while( (U0LSR&0x40)==0 );//Ensures that no data is left to be sent	    
}

//This function sends a string of characters on the serial port
void UART0_SendStr(const unsigned char *str)
{  	while( (U0LSR&0x40)==0 );
   while(1)
   {  
      if( *str == '\0' ) break;
      UART0_SendByte(*str++);	    
   }
}

void Init_Ports(void)
{
//Only simulation in this program
}

void Init_Peripherals(void)
{
 Init_Ports();
 Init_UART0();
}

void DelaymSec(unsigned int j)		  //App 1mSec delay
{  
 unsigned int  i;
 for(;j>0;j--)
 {
  for(i=0; i<10000; i++);
 } 
}

void vnoticer( void * pvParameters )
 {		 
  uint32_t ulNotifiedValue=0x01;

    while(1)
    {	
	
      if( xTaskNotifyWait( 0x00,0xffff,&ulNotifiedValue,1000 )==pdTRUE)  
 {
        
	
		if( ( ulNotifiedValue | 0x01 ) == 0x01 )	   //checking if the received message is same as the sent
  {  UART0_SendStr("Received   MSG from N1 \n");	}

       else if( ( ulNotifiedValue | 0x02 ) == 0x02 )
  {  UART0_SendStr("Received   MSG from N2\n");		}
	   	 
		else if( ( ulNotifiedValue | 0x03 ) == 0x03 )
  {  UART0_SendStr("Received   MSG from N3\n"); 	}
       
       else if( ( ulNotifiedValue | 0x04 ) == 0x04 )
  {   UART0_SendStr("Received   MSG from N4\n");  }

         else
  {   UART0_SendStr("Learn Programming !\n");}
  		
}
		 
else
{ UART0_SendStr("No Notice\n"); }
   
 }	 
}

void vn1( void * pvParameters )
{  

   xHandle = xTaskGetHandle( "Noticer" );
   
   while(1)
   {
   	vTaskDelay(4000);
	UART0_SendStr("N1 sent a Message\n");
	xTaskNotify(xHandle, 0x01, eSetBits);
	
	
	}
}
void vn2( void * pvParameters )
{  

   xHandle = xTaskGetHandle( "Noticer" );
   
   while(1)
   {
   	vTaskDelay(5000);
	UART0_SendStr("N2 sent a Message\n");
	xTaskNotify(xHandle, 0x02, eSetBits);
	
	
	}
}
void vn3( void * pvParameters )
{  

   xHandle = xTaskGetHandle( "Noticer" );
   
   while(1)
   {
   	vTaskDelay(6000);
	UART0_SendStr("N3 sent a MSG\n");
	xTaskNotify(xHandle, 0x03, eSetBits);
	
	
	}
}
void vn4( void * pvParameters )
{  

   xHandle = xTaskGetHandle( "Noticer" );
   
   while(1)
   {
   	vTaskDelay(7000);
	UART0_SendStr("N4 sent a MSG\n");
	xTaskNotify(xHandle, 0x04, eSetBits);
	
	
	}
}


  int main()
{	
 PINSEL0 = 0x00000000;		// Reset all pins as GPIO
 PINSEL1 = 0x00000000;
 PINSEL2 = 0x00000000;
 Init_Peripherals();
	
	
	 
	UART0_SendStr("\t\tMailBox using Task Notification\n");

		xTaskCreate(vn1,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn2,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn3,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn4,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
    	xTaskCreate(vnoticer,"Noticer", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
	
		
		
		
		vTaskStartScheduler();  //Task Scheduling   					    
		
	while(1)//Never reaches this Part of the main
	{
	UART0_SendStr("\t\tMailBox Bypassed\n");
	}

}

