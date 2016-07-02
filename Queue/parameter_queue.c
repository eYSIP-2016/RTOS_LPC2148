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


 
char *tx1={"Task 1 sent a message "};
char *tx2={"Task 2 sent a message"};
char *tx3={"Task 3 sent a message"};
char *tx4={"Task 4 sent a message"};




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

QueueHandle_t xQueue= 0;
 
void Txtask(char *);
void Rxtask(void *);

void Txtask(char *p)			// task which writes data on to the Queue
{
	while(1)
	{
					  
		if(xQueueSend(xQueue,p,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
		{  //vTaskResume("RxTask");
		//Data added to Q
				}				  
		else
		{  	
		}
	//	vTaskDelay(2000);
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
		
	{//	vTaskSuspend(NULL);
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


