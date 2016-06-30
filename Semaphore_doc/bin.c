#include<stdlib.h>
#include"FreeRTOS.h"
#include"task.h"
#include"LCD.h"
#include"semphr.h"

SemaphoreHandle_t xSemaphore;

//Look in the sample programs for Included functions variables etc

void forward(void *pvparam)
{	 vTaskDelay(5);	//Added so that Back Task can occupy the resource first
	while(1)
	{
	 if(xSemaphoreTake(xSemaphore,portMAX_DELAY)==pdTRUE)
	 	{	Stop(); 
			Forward();
		  UART0_SendStr("Forward\n");
		vTaskDelay(5);//To avoid same Tasking Taking resources twice
		}

	
	}
}							  

void back(void *pvparam)
{	 
	while(1)
	{
	 if(xSemaphoreTake(xSemaphore,portMAX_DELAY)==pdTRUE)
	 	{	Stop();
			Back();
		 UART0_SendStr("Back\n");
		vTaskDelay(5);
		}

	}
}							  

void control_switcher(void *pvparam)
{     
  while(1)
  {xSemaphoreGive(xSemaphore);
   UART0_SendStr("Semaphore given\n");
   vTaskDelay(1200);
   
   
   }
}


  int main()
{	
 PINSEL0 = 0x00000000;		// Reset all pins as GPIO
 PINSEL1 = 0x00000000;
 PINSEL2 = 0x00000000;
 DelaymSec(40);
 	Init_Peripherals();
	 
	UART0_SendStr("\t\tBinary Semaphore\n");
    xSemaphore=xSemaphoreCreateBinary( );
		
	xTaskCreate(forward,"forward", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL);//Task Creation
	xTaskCreate(back,"back", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL); //Task Creation
	xTaskCreate(control_switcher,"control_switcher", 300 ,NULL, tskIDLE_PRIORITY + 1, NULL); //Task Creation
	
	vTaskStartScheduler();  //Task Scheduling   					    
	
	while(1);


}	
