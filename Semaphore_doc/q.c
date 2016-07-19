#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
//assuming necessary functions and header files have been included
unsigned char Temp=0;
int count=0;
QueueHandle_t xQueue= 0;
 
void Txtask(char *);
void Rxtask(void *);

 
char *tx1={"Task 1 "};
char *tx2={"Task 2"};
char *tx3={"Task 3"};
char *tx4={"Task 4"};



void Txtask(char *p)			// task which writes data on to the Queue
{
	while(1)
	{
					  
		if(xQueueSend(xQueue,p,1000) == pdTRUE)   	// wait for 1000ms to tx queue message
		{	UART0_SendStr("\nData sent to Queue : \t");
			UART0_SendStr(p);  
		vTaskResume("RxTask");
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
			UART0_SendStr("Data read from Queue : \t");
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
	UART0_SendStr("Queue\n");		
				/* creating the 2 task with the same priority */
	xTaskCreate(Txtask,"TxTask 1", configMINIMAL_STACK_SIZE,tx1, tskIDLE_PRIORITY + 1, NULL);	//Task creation
	xTaskCreate(Txtask,"TxTask 2", configMINIMAL_STACK_SIZE,tx2, tskIDLE_PRIORITY + 1, NULL);	//Task creation
	xTaskCreate(Txtask,"TxTask 3", configMINIMAL_STACK_SIZE,tx3, tskIDLE_PRIORITY + 1, NULL);	//Task creation
	xTaskCreate(Txtask,"TxTask 4", configMINIMAL_STACK_SIZE,tx4, tskIDLE_PRIORITY + 1, NULL);	//Task creation
	xTaskCreate(Rxtask,"RxTask", configMINIMAL_STACK_SIZE,NULL, tskIDLE_PRIORITY + 1, NULL);	//Task creation

    vTaskStartScheduler();     	 /* Start the scheduler so the tasks start executing.  		 */
	
    while(1)
	{
	   ; /* Should never get here!										*/
	}		
}


