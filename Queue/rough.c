/*

Replaced contenets in lpc21xx with that of lpc214x to remove warnings 
Include startup.s from experiments in case of data abort
Counting.axf: Error: L6218E: Undefined symbol pvPortMalloc (referred from tasks.o). error is caused when heap 2 is not included
*/

/************************************************************************ 
Written by: K V S SUMAKAR and KARTIKEYAN V, ERTS Lab, IIT Bombay. Date: 23 June 2016
 IDE: Keil uVision4


Problem:			To solve Polling in serial communication

Concepts covered:	Queues

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
	THIS IS THE MOST EPIC THING IN THIS CODE
	while(1)
	{
	rp=&s1[0];
	 UART0_SendStr(rp);
	 UART0_SendStr("\n");
	}
	*/
#include<stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include"semphr.h"
//#incldue<math.h>
TaskHandle_t xHandle = NULL;
QueueHandle_t xQueue;
#define qpeekNO_BLOCK			( 1000 )
char Temp;

 char *s1={"N1 sent a Message"},*s2={"N2 sent a Message\n"},*s3={"N3 sent a Message\n"},*s4={"N4 sent a Message\n"},*rp;
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


void vn1( void * pvParameters )
{ 
while(1)
   {
   	//vTaskDelay(1000);

	if( xQueueSend( xQueue, s1, qpeekNO_BLOCK ) == pdTRUE )
			{UART0_SendStr("Sent N1"); 
	//	vTaskResume("Sprint");
			}

	else
		{
			UART0_SendStr("Queue push failed");
	//		vTaskDelay(1000);

		}
	}
}
void vn2( void * pvParameters )
{  while(1)
   {
   	vTaskDelay(10);
	if( xQueueSend( xQueue, s2, qpeekNO_BLOCK ) == pdTRUE )
			{UART0_SendStr("Sent N2"); 
	//	vTaskResume("Sprint");
			}

	else
		{
			UART0_SendStr("Queue push failed");
	//		vTaskDelay(1000);

		}
		}
}
void vn3( void * pvParameters )
{     while(1)
   {
   	vTaskDelay(20);
		if( xQueueSend( xQueue, s3, qpeekNO_BLOCK ) == pdTRUE )
			{UART0_SendStr("Sent N3"); 
		//vTaskResume("Sprint");
			}

	else
		{
			UART0_SendStr("Queue push failed");
	//		vTaskDelay(1000);

		}
	}
}
void vn4( void * pvParameters )
{    while(1)
   {
   	vTaskDelay(30);
	if( xQueueSend( xQueue, s4, qpeekNO_BLOCK ) == pdTRUE )
			{UART0_SendStr("Sent N4"); 
	//	vTaskResume("Sprint");
			}

	else
		{
			UART0_SendStr("Queue push failed");
	//		vTaskDelay(1000);

		}	}
}
void vsprint( void * pvParameters )
	 {
	 	 while(1)
		 {
		 while(xQueueReceive( xQueue,rp, qpeekNO_BLOCK ) == pdPASS )
		 	   {UART0_SendStr(rp);
			   UART0_SendStr("\n");
		 		}
		   
		   {  UART0_SendStr("Self Destruct in 3 2 1\n");
		   //	vTaskSuspend(NULL);
	
		   }
		 }
	 
	 }
  int main()
{	
 PINSEL0 = 0x00000000;		// Reset all pins as GPIO
 PINSEL1 = 0x00000000;
 PINSEL2 = 0x00000000;
 Init_Peripherals();
	

	  //xQueueHandle xQueueCreate( unsigned portBASE_TYPE uxQueueLength,unsigned portBASE_TYPE uxItemSize);  
		xQueue=xQueueCreate(10,sizeof(s1));
		UART0_SendStr("\t\tQ\n");

		xTaskCreate(vn1,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn2,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn3,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vn4,"Notifier", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
    	xTaskCreate(vsprint,"Sprint", 300 ,NULL, tskIDLE_PRIORITY + 2, NULL);//Task Creation
		vTaskStartScheduler();  //Task Scheduling   					    
		
	while(1)//Never reaches this Part of the main
	{
	UART0_SendStr("\t\tSeriously ?\n");
	}
	
}

