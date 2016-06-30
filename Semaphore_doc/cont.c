/* 
Note: To use mutex semaphore you need to initialize  configUSE_MUTEXES to 1 
*/

#include<stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "LCD.h"
#include"semphr.h"

SemaphoreHandle_t xSemaphore=0;//Creation of Variable for semaphore

int s=0; 
int forks_avail[5]={0,0,0,0,0}; //The value of Variable is 0 if a fork is available at that location and 1 if the fork is taken


void vfork( void * pvParameters )
{	 
	int i; 											//For ith Fork
	const unsigned char* str;						//Pointer for the passed Parameter
	str = ( const unsigned char * ) pvParameters; 
				 
				
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
					
		 
		UART0_SendStr(&str[0]);	
		UART0_SendStr(":Right fork obtained\n");
					 	
	if(( xSemaphoreTake( xSemaphore, 2000 ) == pdTRUE )&&(forks_avail[(i+1)%5]==0))
	{ //Waits for 2000 ticcks for Left fork to be available
								   
			forks_avail[(i+1)%5]=1;
										
			UART0_SendStr(&str[0]);
			UART0_SendStr(":Left fork obtained Eating :)\n");
										
			vTaskDelay(2000);
			UART0_SendStr(&str[0]);
			UART0_SendStr(":Ate \n");
		   		 
			xSemaphoreGive(xSemaphore);
			xSemaphoreGive(xSemaphore);
			forks_avail[i]=0;	 
		 	forks_avail[(i+1)%5]=0;
		    UART0_SendStr(&str[0]);
			UART0_SendStr(":Thinking \n");
			vTaskDelay(3000);
}

		else
		{ 		  
	    UART0_SendStr(&str[0]);
		UART0_SendStr(":Returned Right fork:( \n");  
		xSemaphoreGive(xSemaphore);
		forks_avail[i]=0;
		}
				  
	}

   else
	   {				    
	   UART0_SendStr(&str[0]);
	   UART0_SendStr(":Hungry\n"); 
	   vTaskDelay(3000);
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

	xSemaphore = xSemaphoreCreateCounting( 5, 5 );
		  
	 if( xSemaphore != NULL )
    {	UART0_SendStr("\tSemaphore Created\n");
        
											
xTaskCreate(vfork,"Philospher 1", 300 ,"P1", tskIDLE_PRIORITY + 1, NULL);//Task Creation
xTaskCreate(vfork,"Philospher 2", 300 ,"P2", tskIDLE_PRIORITY + 1, NULL);//Task Creation
xTaskCreate(vfork,"Philospher 3", 300 ,"P3", tskIDLE_PRIORITY + 1, NULL);//Task Creation
xTaskCreate(vfork,"Philospher 4", 300 ,"P4", tskIDLE_PRIORITY + 1, NULL);//Task Creation
xTaskCreate(vfork,"Philospher 5", 300 ,"P5", tskIDLE_PRIORITY + 1, NULL);//Task Creation
		
		vTaskStartScheduler();  //Task Scheduling   					    
     }
		
	while(1)//Never reaches this Part of the main
	{UART0_SendStr("\t\tSemaphore not Created\n");	}

}

