#include<stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "LCD.h"
#include"semphr.h"

//Refer to actual code for necessary functions and codes

SemaphoreHandle_t xSemaphore=0;//Creation of Variable for semaphore

  void forward(void *pvparam)
{
	while(1)
	{
		if(xSemaphoreTake(xSemaphore,1000) == pdTRUE )	  // if available then 
		{   UART0_SendStr("Forward\n");
		   	Forward();
		    vTaskDelay(1200);			// task suspended for 1200ms
			Stop();
			xSemaphoreGive( xSemaphore );		    // after resource task completed, return the semaphore
		}
		
		else
		{	UART0_SendStr("Forward function access denied\n"); 	}
		
		 vTaskDelay(200);							// try again after delay of 200ms
	}
}							  

void back(void *pvparam)
{	 while(1)
	{
			if(xSemaphoreTake(xSemaphore,1000) == pdTRUE )
			{
			UART0_SendStr("Back\n");
			Back();
			vTaskDelay(1200);			// perform other shared  data tasks();
			Stop();	 
			xSemaphoreGive( xSemaphore );			   // after shared data task completed, return the semaphore
	}
	
	else	   // if available then 
		{ UART0_SendStr("Back Function access denied\n");		}
		
	vTaskDelay(200);						// try again after delay of 1000ms
}
}

  int main()
{	
 PINSEL0 = 0x00000000;		// Reset all pins as GPIO
 PINSEL1 = 0x00000000;
 PINSEL2 = 0x00000000;
 DelaymSec(40);
 	Init_Peripherals();
	 
	UART0_SendStr("\t\tMutex\n");
	xSemaphore = xSemaphoreCreateMutex();	//Use the Handle as a MUTEX	  
	
	xTaskCreate(forward,"forward", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
	xTaskCreate(back,"back", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL); //Task Creation
	
	vTaskStartScheduler();  //Task Scheduling   					    
	
	while(1);
}
