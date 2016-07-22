			   /*

Replaced contenets in lpc21xx with that of lpc214x to remove warnings 
Include startup.s from experiments in case of data abort
Counting.axf: Error: L6218E: Undefined symbol pvPortMalloc (referred from tasks.o). error is caused when heap 2 is not included
*/

/************************************************************************ 
Written by: K V S SUMAKAR and KARTIKEYAN V, ERTS Lab, IIT Bombay. Date: 23 June 2016
 IDE: Keil uVision4


Problem:To implement Dining Philosopher problem using counting semaphore

Concepts covered:	
Counting semaphores


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




Declaration of Counting
*/
//****************************************************************************************************/
/* 
Note: To use mutex semaphore you need to initialize  configUSE_MUTEXES to 1 
*/

#include<stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "LCD.h"
#include"semphr.h"
#include<string.h>
SemaphoreHandle_t xSemaphore=0;//Creation of Variable for semaphore
QueueHandle_t xQueue= 0;

unsigned char Temp=0;
int s=0; 
int forks_avail[5]={0,0,0,0,0}; //The value of Variable is 0 if a fork is available at that location and 1 if the fork is taken

char *s1={"\nRight fork obtained by:"};
char *s2={"\nLeft fork obtained Eating :"};
char *s3={"\nAte :"};
char *s4={"\nThinking :"};
char *s5={"\nRight fork Returned by:"}; 
char *s6={"\nHungry :"};

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

void vfork( void * pvParameters )
			{	 
			int i; 											//For ith Fork
			const unsigned char* str;	
			char *t1,*t2,*t3,*t4,*t5,*t6;
			str = ( const unsigned char * ) pvParameters; 
			strcpy(t1,str);						
			strcpy(t2,str);
			strcpy(t3,str);
			strcpy(t4,str);
			strcpy(t5,str);
			strcpy(t6,str);		
			
			strcat(t1,s1);
			strcat(t2,s2);
			strcat(t3,s3);
			strcat(t4,s4);
			strcat(t5,s5);
			strcat(t6,s6);					
		//Pointer for the passed Parameter
			
				 
				
				//Assignment of forks available on the basis of name of Philospher
				if(str[1]=='1')
				{i=0;}
				if(str[1]=='2')
				{i=1;}
				if(str[1]=='3')
				{i=2;}
				if(str[1]=='4')
				{i=3;}
				if(str[1]=='5')
				{i=4;}
				 
				while(1)
				{  
				//Waits for 1000 ticks for forks to be avaliable
				//If available checks if the fork is adjacent(Right) or not
				if(( xSemaphoreTake( xSemaphore, 1000 ) == pdTRUE )&&(forks_avail[i]==0))		  	{
					forks_avail[i]=1;
					
				if(xQueueSend(xQueue,t1,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
						{  vTaskResume("RxTask"); 			}	
					 	   //UART0_SendStr(s1);
						if(( xSemaphoreTake( xSemaphore, 2000 ) == pdTRUE )&&(forks_avail[(i+1)%5]==0))
				  				   { //Waits for 2000 ticcks for Left fork to be available
								   
				  						 	forks_avail[(i+1)%5]=1;
											
											
											if(xQueueSend(xQueue,t2,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
											{  vTaskResume("RxTask");	}	
											vTaskDelay(2000);
											  //UART0_SendStr(s2);
											
											if(xQueueSend(xQueue,t3,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
									{  vTaskResume("RxTask");	}
									//UART0_SendStr(s3);	
									   		while( (U0LSR&0x40)==0 );
											xSemaphoreGive(xSemaphore);
									 		xSemaphoreGive(xSemaphore);
											forks_avail[i]=0;	 
										 	forks_avail[(i+1)%5]=0;
											
												
											if(xQueueSend(xQueue,t4,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
											{  vTaskResume("RxTask");}
										//	UART0_SendStr(s4);	
										
											vTaskDelay(3000);
																				}

						else
								{ 
								
								if(xQueueSend(xQueue,t5,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
								{  vTaskResume("RxTask");	}	
							//	UART0_SendStr(s5);
								xSemaphoreGive(xSemaphore);
								forks_avail[i]=0;
								}
				  
				  	}

				   else
				   {
				   
					if(xQueueSend(xQueue,t6,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
					{  vTaskResume("RxTask");}
				//	UART0_SendStr(s6);	
				   while( (U0LSR&0x40)==0 );
				   vTaskDelay(3000);
					}
				}
				 }
			

void Rxtask(void *p)			// task which reads data from the Queue
{

unsigned char rx_success_count[11]={0};
unsigned char *rxptr;
rxptr = rx_success_count;

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
 PINSEL0 = 0x00000000;		// Reset all pins as GPIO
 PINSEL1 = 0x00000000;
 PINSEL2 = 0x00000000;
 Init_Peripherals();
	
	
	 
	UART0_SendStr("\t\tCounting Semaphore\n");

	xSemaphore = xSemaphoreCreateCounting( 5, 5 );	//Use the Handle as a counting semaphore	  
	xQueue = xQueueCreate(10,40);
	
	  
	 if( xSemaphore != NULL )
    {	UART0_SendStr("\tSemaphore Created\n");
        
											
		xTaskCreate(vfork,"Philospher 1", 300 ,"P1", tskIDLE_PRIORITY + 1, NULL);//Task Creation
    	xTaskCreate(vfork,"Philospher 2", 300 ,"P2", tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vfork,"Philospher 3", 300 ,"P3", tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vfork,"Philospher 4", 300 ,"P4", tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(vfork,"Philospher 5", 300 ,"P5", tskIDLE_PRIORITY + 1, NULL);//Task Creation
		xTaskCreate(Rxtask,"RxTask", configMINIMAL_STACK_SIZE,NULL, tskIDLE_PRIORITY + 1, NULL);

		
		
		vTaskStartScheduler();  //Task Scheduling   					    
     }
		
	while(1)//Never reaches this Part of the main
	{
	UART0_SendStr("\t\tSemaphore not Created\n");
	}

}

