/*

Replaced contenets in lpc21xx with that of lpc214x to remove warnings 
Include startup.s from experiments in case of data abort
Counting.axf: Error: L6218E: Undefined symbol pvPortMalloc (referred from tasks.o). error is caused when heap 2 is not included
*/

/************************************************************************ 
Written by: K V S SUMAKAR and KARTIKEYAN V, ERTS Lab, IIT Bombay. Date: 23 June 2016
 IDE: Keil uVision4


Problem:To use queue to solve polling of Queue

Concepts covered:	
Queue,Parameterised Tasks,MultiTasking etc

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
 
	 set configUSE_COUNTING_SEMAPHORES to 1	 to use counting semaphore
	For more details refer section 4.8 in the hardware manual.




Declaration of Queue :
QueueHandle_t xQueue= 0;
  xQueue = xQueueCreate(length,size of each element);
	

*/
//****************************************************************************************************/
/* 
Note: To use mutex semaphore you need to initialize  configUSE_MUTEXES to 1 
*/



#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
//#include "lcd.h"

#define Fosc            12000000                    //10MHz~25MHz
#define Fcclk           (Fosc * 5)                  //Fosc(1~32)<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO Fcclk 2񅦰�16�156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPB(Fcclk / 4) 1񄿔
#define  UART_BPS	9600 		//Change Baud Rate Setting here

unsigned char Temp=0;
int count=0;

void Init_UART0(void);
void  __irq IRQ_UART0(void);
void UART0_SendByte(unsigned char data);
void UART0_SendStr(const unsigned char *str);


 
char *tx1={"Task 1 sent a message"};
char *tx2={"Task 2 sent a message"};
char *tx3={"Task 3 sent a message"};
char *tx4={"Task 4 sent a message"};


QueueHandle_t xQueue= 0;
 
void Txtask(char *);
void Rxtask(void *);



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

void Txtask(char *p)			// task which writes data on to the Queue
{
	while(1)
	{
					  
		if(xQueueSend(xQueue,p,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
		{  vTaskResume("RxTask");
		//Data added to Q
		}				  
		
	}
	
}

void Rxtask(void *p)			// task which reads data from the Queue
{

unsigned char *rxptr;

	while(1)
	{
		if(xQueueReceive(xQueue,rxptr,1000) == pdTRUE)		// wait for 1000ms to rx queue message
		{  	UART0_SendStr("\n");
			UART0_SendStr(rxptr);
			vTaskDelay(40);			   // if RX success then display  rx_success_count
		}
		else  
		
	{	vTaskSuspend(NULL);
	}	
	}
}



int main()
{  	 
 Init_UART0();
 	
				 /*create queue	of length=3 and of size int*/
	xQueue = xQueueCreate(7,40);
			
				/* creating the 2 task with the same priority */
	xTaskCreate(Txtask,"TxTask 1", configMINIMAL_STACK_SIZE,tx1, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(Txtask,"TxTask 2", configMINIMAL_STACK_SIZE,tx2, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(Txtask,"TxTask 3", configMINIMAL_STACK_SIZE,tx3, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(Txtask,"TxTask 4", configMINIMAL_STACK_SIZE,tx4, tskIDLE_PRIORITY + 1, NULL);	
	xTaskCreate(Rxtask,"RxTask", configMINIMAL_STACK_SIZE,NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();     	 /* Start the scheduler so the tasks start executing.  		 */
	
    while(1)
	{
	   ; /* Should never get here!										*/
	}		
}


